package monitor // import "github.com/AntiBargu/lkp-extent/lkp-master/monitor"

import (
	"go.uber.org/zap"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/job"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/node"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/task"

	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/jobdb"
	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/nodedb"
	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/taskdb"
)

type Monitor struct {
	Cfg    *Config
	Logger *zap.Logger
	Nodes  node.Store
	Jobs   job.Store
	Tasks  task.Store
}

// NewMonitor sets up monitor to be able to service requests from LKP-Nodes.
func NewMonitor(cfg []byte) *Monitor {
	return &Monitor{
		Cfg:   loadConfig(cfg),
		Nodes: nodedb.GetNodeDB(),
		Jobs:  jobdb.GetJobDB(),
		Tasks: taskdb.GetTaskDB(),
	}
}
