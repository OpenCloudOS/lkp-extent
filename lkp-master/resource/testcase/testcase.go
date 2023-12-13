package testcase // import "github.com/AntiBargu/lkp-extent/lkp-master/resource/testcase"

import (
	"time"

	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/stringid"
)

// Here is a concurrency safety issues
type Testcase struct {
	ID    string
	Name  string
	Tag   string
	Path  string
	CTime time.Time
	ATime time.Time
	MTime time.Time
	Size  uint64
}

func NewTestcase() *Testcase {
	return &Testcase{
		ID:    stringid.GenerateRandomID(),
		CTime: time.Now(),
		ATime: time.Now(),
		MTime: time.Now(),
	}
}

type Store interface {
	// Add appends a new Testcase to the store.
	Add(*Testcase) error

	// GetByID returns a Testcase from the store by the ID it was stored with.
	GetByID(string) (*Testcase, error)
	// GetByNameTag returns a Testcase from the store by Name:Tag it was stored with.
	GetByNameTag(string, string) (*Testcase, error)
	// GetByPrefix returns a Testcase from the store by the ID Prefix it was stored with.
	GetByPrefix(string) (*Testcase, error)

	// IsExistingID returns true if the ID exists
	IsExistingID(string) bool
	// IsExistingNameTag returns true if Name:Tag exists
	IsExistingNameTag(string, string) bool

	// List returns a list of Testcases from the store.
	List() []*Testcase
}
