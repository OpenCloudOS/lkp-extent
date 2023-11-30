package monitor // import "github.com/AntiBargu/lkp-extent/lkp-master/monitor"

import (
	"context"
	"fmt"
	"os"
	"time"

	"go.uber.org/zap"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/job"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/task"

	mgrpc "github.com/AntiBargu/lkp-extent/api/monitor"
)

func (m *Monitor) TaskFinish(ctx context.Context, req *mgrpc.TaskFinishRequest) (*mgrpc.TaskFinishResponse, error) {
	resp := &mgrpc.TaskFinishResponse{}

	m.Logger.Debug("task finish request", zap.String("Task ID", req.ID))

	t, err := m.Tasks.GetByPrefix(req.ID)
	if err != nil {
		m.Logger.Info("Unable to get task", zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "Task %q: doesn't exist", req.ID)
	}

	j, err := m.Jobs.GetByPrefix(t.JobID)
	if err != nil {
		m.Logger.Info("Unable to get job", zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "Job %q: doesn't exist", req.ID)
	}

	if len(req.File) == 0 {
		t.Stat = task.ERROR
		t.FTime = time.Now()
	} else {
		if err := os.MkdirAll(fmt.Sprintf(m.Cfg.Resource.Task.Path+"%s", req.ID), 0755); err != nil {
			m.Logger.Info("Create result directory failed", zap.Error(err))
			return resp, status.Errorf(codes.Internal, "Create result directory failed: %q: %v",
				fmt.Sprintf(m.Cfg.Resource.Task.Path+"%s", req.ID), err)
		}
		rsltPath := fmt.Sprintf(m.Cfg.Resource.Task.Path+"%s/rslt.tar.gz", req.ID)
		if err := os.WriteFile(rsltPath, req.File, 0644); err != nil {
			m.Logger.Info("Result write failed", zap.Error(err))
			return resp, status.Errorf(codes.Internal, "Result write failed: %q: %v", rsltPath, err)
		}

		t.Stat = task.FINISH
		t.FTime = time.Now()
		t.RsltPath = rsltPath
	}

	j.FCnt += 1
	if j.FCnt == int64(len(j.Tasks)) {
		j.FTime = time.Now()
		j.Stat = job.FINISH
	}

	return resp, nil
}
