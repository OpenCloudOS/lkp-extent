package daemon // import "github.com/AntiBargu/lkp-extent/lkp-master/daemon"

import (
	"go.uber.org/zap"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/job"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/node"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/task"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/testcase"

	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/casedb"
	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/jobdb"
	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/nodedb"
	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/taskdb"
)

type Daemon struct {
	Cfg       *Config
	Logger    *zap.Logger
	nodes     node.Store
	testcases testcase.Store
	jobs      job.Store
	tasks     task.Store
}

// NewDaemon sets up daemon to be able to service requests from the CLI.
func NewDaemon(cfg []byte) *Daemon {
	return &Daemon{
		Cfg:       loadConfig(cfg),
		nodes:     nodedb.GetNodeDB(),
		testcases: casedb.GetCaseDB(),
		jobs:      jobdb.GetJobDB(),
		tasks:     taskdb.GetTaskDB(),
	}
}
