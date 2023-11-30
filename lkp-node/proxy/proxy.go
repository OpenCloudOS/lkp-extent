package proxy // import "github.com/AntiBargu/lkp-extent/lkp-node/proxy"

import (
	"context"
	"fmt"
	"os"
	"path/filepath"

	"go.uber.org/zap"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/AntiBargu/lkp-extent/lkp-node/pkg/taskqueue"
	"github.com/AntiBargu/lkp-extent/lkp-node/task"

	schedgrpc "github.com/AntiBargu/lkp-extent/api/sched"
)

type Proxy struct {
	Cfg       *Config
	Logger    *zap.Logger
	TaskQueue *taskqueue.TaskQueue
}

func NewProxy(cfg []byte) *Proxy {
	return &Proxy{
		Cfg: loadConfig(cfg),
	}
}

func (p *Proxy) CreateWorkingDirs() {
	p.Logger.Debug("Creating Proxy working dirs")

	dirs := []string{
		p.Cfg.Testcase.Path,
	}

	for _, dir := range dirs {
		err := os.MkdirAll(dir, 0755)
		if err != nil {
			p.Logger.Fatal("Proxy environment initialization failed", zap.Error(err))
		}
	}
}

func (p *Proxy) CasePush(ctx context.Context, req *schedgrpc.CasePushRequest) (*schedgrpc.CasePushResponse, error) {
	resp := &schedgrpc.CasePushResponse{}

	p.Logger.Debug("case push request", zap.String("Case ID", req.ID), zap.String("Name", req.Name))

	casePath := fmt.Sprintf(p.Cfg.Testcase.Path+"%s/%s.yaml", req.ID, req.Name)
	if err := os.MkdirAll(filepath.Dir(casePath), os.ModePerm); err != nil {
		p.Logger.Info("Create directory failed", zap.Error(err))
		return resp, status.Errorf(codes.Internal, "Create directory failed: %q: %v", filepath.Dir(casePath), err)
	}
	if err := os.WriteFile(casePath, req.File, 0644); err != nil {
		p.Logger.Info("Testcase write failed", zap.Error(err))
		return resp, status.Errorf(codes.Internal, "Testcase write failed: %v", err)
	}

	return resp, nil
}

func (p *Proxy) JobRun(ctx context.Context, req *schedgrpc.JobRunRequest) (*schedgrpc.JobRunResponse, error) {
	resp := &schedgrpc.JobRunResponse{}

	p.Logger.Debug("Job run request", zap.String("Case ID", req.CaseID), zap.String("Case Name", req.CaseName))

	filePath := fmt.Sprintf(p.Cfg.Testcase.Path+"%s/%s.yaml", req.CaseID, req.CaseName)
	if _, err := os.Stat(filePath); err != nil {
		return resp, status.Errorf(codes.Internal, "%q doesn't exist %v", req.CaseID, err)
	}

	for _, item := range req.Tasks {
		p.TaskQueue.EnQueue(&task.Task{
			ID:       item.ID,
			Role:     int32(item.Role),
			CaseID:   req.CaseID,
			CaseName: req.CaseName,
		})
	}

	return resp, nil
}
