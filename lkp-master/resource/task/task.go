package task // import "github.com/AntiBargu/lkp-extent/lkp-master/resource/task"

import (
	"time"

	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/stringid"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/node"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/testcase"
)

const (
	STANDBY = iota
	RUNNING
	FINISH
	ERROR
)

const (
	HOST = iota + 1
	CONTAINER
)

type Task struct {
	ID          string
	JobID       string
	Stat        int32
	CTime       time.Time
	FTime       time.Time
	Role        int32
	RsltPath    string
	ContainerID string
	Tc          *testcase.Testcase
	Node        *node.Node
}

func NewTask() *Task {
	return &Task{
		ID:    stringid.GenerateRandomID(),
		Stat:  STANDBY,
		CTime: time.Now(),
	}
}

type Store interface {
	// Add appends a new Task to the store.
	Add(*Task) error

	// GetByID returns a Task from the store by the ID it was stored with.
	GetByID(string) (*Task, error)
	// GetByPrefix returns a Task from the store by the ID Prefix it was stored with.
	GetByPrefix(string) (*Task, error)

	// IsExistingID returns true if the ID exists
	IsExistingID(string) bool

	// List returns a list of Task from the store.
	List() []*Task
}
