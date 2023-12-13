package job // import "github.com/AntiBargu/lkp-extent/lkp-master/resource/job"

import (
	"sync"
	"time"

	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/namegenerator"
	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/stringid"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/task"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/testcase"
)

const (
	STANDBY = iota
	RUNNING
	FINISH
	ERROR
)

type Job struct {
	ID    string
	Name  string
	Stat  int32
	FCnt  int64
	FLock sync.Mutex
	CTime time.Time
	FTime time.Time
	Tc    *testcase.Testcase
	Tasks []*task.Task
}

func NewJob(name string) *Job {
	rslt := &Job{
		ID:    stringid.GenerateRandomID(),
		Name:  name,
		Stat:  STANDBY,
		CTime: time.Now(),
		Tasks: []*task.Task{},
	}

	if rslt.Name == "" {
		rslt.Name = namegenerator.GetRandomName(0)
	}

	return rslt
}

type Store interface {
	// Add appends a new Job to the store.
	Add(*Job) error

	// GetByID returns a Job from the store by the ID it was stored with.
	GetByID(string) (*Job, error)
	// GetByName returns a Job from the store by Name it was stored with.
	GetByName(string) (*Job, error)
	// GetByPrefix returns a Job from the store by the ID Prefix it was stored with.
	GetByPrefix(string) (*Job, error)

	// IsExistingID returns true if the ID exists
	IsExistingID(string) bool
	// IsExistingName returns true if the Name exists
	IsExistingName(string) bool

	// List returns a list of Job from the store.
	List() []*Job
}
