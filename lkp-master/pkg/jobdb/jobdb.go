package jobdb // import "github.com/AntiBargu/lkp-extent/lkp-master/pkg/jobdb"

import (
	"fmt"
	"log"
	"os"
	"strings"
	"sync"

	yaml "gopkg.in/yaml.v2"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/job"
	"github.com/AntiBargu/lrulist"
)

type config struct {
	Resource struct {
		Job struct {
			Limit int `yaml:"limit" json:"limit"`
		} `yaml:"job" json:"job"`
	} `yaml:"resource" json:"resource"`
}

type jobDB struct {
	list *lrulist.LRUList
	// prefixTbl map[string]map[string]string
	prefixTbl map[string][]string
	nameTbl   map[string]string
	cap       int
	lock      sync.RWMutex
}

const (
	limit   = 1024
	cfgFile = "/etc/lkp/cfg.yaml"
)

var db *jobDB

func init() {
	data, err := os.ReadFile(cfgFile)
	if err != nil {
		log.Fatal(err)
	}

	cfg := &config{}
	yaml.Unmarshal(data, cfg)

	cap := cfg.Resource.Job.Limit
	if cap == 0 {
		cap = limit
	}

	db = &jobDB{
		prefixTbl: make(map[string][]string),
		nameTbl:   make(map[string]string),
		cap:       cap,
		lock:      sync.RWMutex{},

		list: lrulist.NewLRUList(cap, func(item interface{}) error {
			job := item.(*job.Job)

			for idx, item := range db.prefixTbl[job.ID[:2]] {
				if item == job.ID {
					db.prefixTbl[job.ID[:2]][idx] = db.prefixTbl[job.ID[:2]][len(db.prefixTbl[job.ID[:2]])-1]
					db.prefixTbl[job.ID[:2]] = db.prefixTbl[job.ID[:2]][:len(db.prefixTbl[job.ID[:2]])-1]
					break
				}
			}

			delete(db.nameTbl, job.Name)

			return nil
		}),
	}
}

func GetJobDB() *jobDB {
	return db
}

func (db *jobDB) Add(job *job.Job) error {
	if db.IsExistingID(job.ID) {
		return fmt.Errorf("id: %q already exists", job.ID)
	}

	if db.IsExistingName(job.Name) {
		return fmt.Errorf("%s: already exists", job.Name)
	}

	db.lock.Lock()
	defer db.lock.Unlock()

	db.list.Set(job.ID, job)

	db.prefixTbl[job.ID[:2]] = append(db.prefixTbl[job.ID[:2]], job.ID)
	db.nameTbl[job.Name] = job.ID

	return nil
}

func (db *jobDB) GetByID(id string) (*job.Job, error) {
	if id == "" {
		return nil, fmt.Errorf("id can't be empty")
	}

	if len(id) < 2 {
		return nil, fmt.Errorf("id is too short")
	}

	db.lock.Lock()
	defer db.lock.Unlock()

	rslt, err := db.list.Get(id)
	if err != nil {
		return nil, err
	}

	return rslt.(*job.Job), nil
}

func (db *jobDB) GetByName(name string) (*job.Job, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	id, exists := db.nameTbl[name]
	if !exists {
		return nil, fmt.Errorf("%s no such job", name)
	}

	rslt, err := db.list.Get(id)
	// Fatal BUG occurs
	if err != nil {
		return nil, err
	}

	return rslt.(*job.Job), nil
}

func (db *jobDB) GetByPrefix(prefix string) (*job.Job, error) {
	if prefix == "" {
		return nil, fmt.Errorf("prefix can't be empty")
	}

	if len(prefix) < 2 {
		return nil, fmt.Errorf("prefix is too short")
	}

	id := ""

	db.lock.RLock()
	defer db.lock.RUnlock()

	for _, item := range db.prefixTbl[prefix[:2]] {
		if strings.HasPrefix(item, prefix) {
			if id != "" {
				return nil, fmt.Errorf("multiple jobs found with provided prefix: %q", prefix)
			}
			id = item
		}
	}

	if id != "" {
		rslt, err := db.list.Get(id)
		// Fatal BUG occurs
		if err != nil {
			return nil, err
		}

		return rslt.(*job.Job), nil
	}

	return nil, fmt.Errorf("%q: no such job", prefix)
}

func (db *jobDB) IsExistingID(id string) bool {
	db.lock.RLock()
	defer db.lock.RUnlock()

	for _, item := range db.prefixTbl[id[:2]] {
		if item == id {
			return true
		}
	}

	return false
}

func (db *jobDB) IsExistingName(name string) bool {
	db.lock.RLock()
	defer db.lock.RUnlock()

	_, exists := db.nameTbl[name]

	return exists
}

func (db *jobDB) List() []*job.Job {
	jobs := []*job.Job{}

	db.lock.RLock()
	defer db.lock.RUnlock()

	db.list.Traverse(func(item interface{}) error {
		job := item.(*job.Job)
		jobs = append(jobs, job)

		return nil
	})

	return jobs
}
