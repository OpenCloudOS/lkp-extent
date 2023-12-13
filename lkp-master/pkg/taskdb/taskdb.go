package taskdb // import "github.com/AntiBargu/lkp-extent/lkp-master/pkg/taskdb"

import (
	"fmt"
	"log"
	"os"
	"strings"
	"sync"

	yaml "gopkg.in/yaml.v2"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/task"
	"github.com/AntiBargu/lrulist"
)

type config struct {
	Resource struct {
		Task struct {
			Limit int `yaml:"limit" json:"limit"`
		} `yaml:"task" json:"task"`
	} `yaml:"resource" json:"resource"`
}

type taskDB struct {
	list *lrulist.LRUList
	// prefixTbl map[string]map[string]string
	prefixTbl map[string][]string
	cap       int
	lock      sync.RWMutex
}

const (
	limit   = 65536
	cfgFile = "/etc/lkp/cfg.yaml"
)

var db *taskDB

func init() {
	data, err := os.ReadFile(cfgFile)
	if err != nil {
		log.Fatal(err)
	}

	cfg := &config{}
	yaml.Unmarshal(data, cfg)

	cap := cfg.Resource.Task.Limit
	if cap == 0 {
		cap = limit
	}

	db = &taskDB{
		prefixTbl: make(map[string][]string),
		cap:       cap,
		lock:      sync.RWMutex{},

		list: lrulist.NewLRUList(cap, func(item interface{}) error {
			task := item.(*task.Task)

			for idx, item := range db.prefixTbl[task.ID[:2]] {
				if item == task.ID {
					db.prefixTbl[task.ID[:2]][idx] = db.prefixTbl[task.ID[:2]][len(db.prefixTbl[task.ID[:2]])-1]
					db.prefixTbl[task.ID[:2]] = db.prefixTbl[task.ID[:2]][:len(db.prefixTbl[task.ID[:2]])-1]
					break
				}
			}

			return nil
		}),
	}
}

func GetTaskDB() *taskDB {
	return db
}

func NewTaskDB(cap int) *taskDB {
	db := &taskDB{
		prefixTbl: make(map[string][]string),
		cap:       cap,
		lock:      sync.RWMutex{},
	}

	db.list = lrulist.NewLRUList(cap, func(item interface{}) error {
		task := item.(*task.Task)

		for idx, item := range db.prefixTbl[task.ID[:2]] {
			if item == task.ID {
				db.prefixTbl[task.ID[:2]][idx] = db.prefixTbl[task.ID[:2]][len(db.prefixTbl[task.ID[:2]])-1]
				db.prefixTbl[task.ID[:2]] = db.prefixTbl[task.ID[:2]][:len(db.prefixTbl[task.ID[:2]])-1]
				break
			}
		}

		return nil
	})

	return db
}

func (db *taskDB) Add(task *task.Task) error {
	if db.IsExistingID(task.ID) {
		return fmt.Errorf("id: %q already exists", task.ID)
	}

	db.lock.Lock()
	defer db.lock.Unlock()

	db.list.Set(task.ID, task)

	db.prefixTbl[task.ID[:2]] = append(db.prefixTbl[task.ID[:2]], task.ID)

	return nil
}

func (db *taskDB) GetByID(id string) (*task.Task, error) {
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

	return rslt.(*task.Task), nil
}

func (db *taskDB) GetByPrefix(prefix string) (*task.Task, error) {
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
				return nil, fmt.Errorf("multiple tasks found with provided prefix: %q", prefix)
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

		return rslt.(*task.Task), nil
	}

	return nil, fmt.Errorf("%q: no such task", prefix)
}

func (db *taskDB) IsExistingID(id string) bool {
	db.lock.RLock()
	defer db.lock.RUnlock()

	for _, item := range db.prefixTbl[id[:2]] {
		if item == id {
			return true
		}
	}

	return false
}

func (db *taskDB) List() []*task.Task {
	tasks := []*task.Task{}

	db.lock.RLock()
	defer db.lock.RUnlock()

	db.list.Traverse(func(item interface{}) error {
		task := item.(*task.Task)
		tasks = append(tasks, task)

		return nil
	})

	return tasks
}
