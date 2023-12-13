package worker // import "github.com/AntiBargu/lkp-extent/lkp-node/worker"

import (
	"context"
	"fmt"
	"os"

	"go.uber.org/zap"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"

	"github.com/AntiBargu/lkp-extent/lkp-node/informer"
	"github.com/AntiBargu/lkp-extent/lkp-node/pkg/taskqueue"

	mgrpc "github.com/AntiBargu/lkp-extent/api/monitor"
)

type Worker struct {
	Token     string
	Cfg       *Config
	Logger    *zap.Logger
	TaskQueue *taskqueue.TaskQueue
}

func NewWorker(cfg []byte) *Worker {
	return &Worker{
		Cfg: loadConfig(cfg),
	}
}

func (w *Worker) CreateWorkingDirs() {
	w.Logger.Debug("Creating Proxy working dirs")

	dirs := []string{
		w.Cfg.Testcase.Path,
		w.Cfg.Result.Path,
	}

	for _, dir := range dirs {
		err := os.MkdirAll(dir, 0755)
		if err != nil {
			w.Logger.Fatal("Proxy environment initialization failed", zap.Error(err))
		}
	}
}
func (w *Worker) Login() {
	con, err := grpc.Dial(w.Cfg.LKPMaster.Monitor.IP+":"+w.Cfg.LKPMaster.Monitor.Port, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		w.Logger.Fatal("Unable to connect to LKP-Master", zap.Error(err))
	}
	defer con.Close()

	mc := mgrpc.NewMonitorClient(con)

	req := &mgrpc.ConnectRequest{
		KernelVer: informer.KernelVersion(),
		DistVer:   informer.Distribution(),
		Arch:      informer.CPUInfo(),
		SchedPort: w.Cfg.LKPNode.Proxy.Port,
		Stat: &mgrpc.StatusMetadata{
			FreeMem:   string(fmt.Sprintf("%3.2fGB", float64(informer.MemorySize())/(1024*1024*1024))),
			DiskUsage: "0",
		},
	}

	resp, err := mc.Connect(context.Background(), req)
	if err != nil {
		w.Logger.Fatal("Unable to connect to LKP-Master", zap.Error(err))
	}

	w.Token = resp.ID
}

func (w *Worker) Logout() {
	con, err := grpc.Dial(w.Cfg.LKPMaster.Monitor.IP+":"+w.Cfg.LKPMaster.Monitor.Port, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		w.Logger.Fatal("Unable to connect to LKP-Master", zap.Error(err))
	}
	defer con.Close()

	mc := mgrpc.NewMonitorClient(con)

	req := &mgrpc.DisconnectRequest{
		ID: w.Token,
	}

	if _, err := mc.Disconnect(context.Background(), req); err != nil {
		w.Logger.Fatal("Unable to connect to LKP-Master", zap.Error(err))
	}
}
