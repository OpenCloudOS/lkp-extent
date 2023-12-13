package casedb // import "github.com/AntiBargu/lkp-extent/lkp-master/pkg/casedb"

import (
	"fmt"
	"log"
	"os"
	"strings"
	"sync"

	yaml "gopkg.in/yaml.v2"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/testcase"
	"github.com/AntiBargu/lrulist"
)

type config struct {
	Resource struct {
		Case struct {
			Limit int `yaml:"limit" json:"limit"`
		} `yaml:"testcase" json:"testcase"`
	} `yaml:"resource" json:"resource"`
}

type caseDB struct {
	list *lrulist.LRUList
	// prefixTbl map[string]map[string]string
	prefixTbl map[string][]string
	nameTbl   map[string]string
	cap       int
	lock      sync.RWMutex
}

const (
	limit   = 128
	cfgFile = "/etc/lkp/cfg.yaml"
)

var db *caseDB

func init() {
	data, err := os.ReadFile(cfgFile)
	if err != nil {
		log.Fatal(err)
	}

	cfg := &config{}
	yaml.Unmarshal(data, cfg)

	cap := cfg.Resource.Case.Limit
	if cap == 0 {
		cap = limit
	}

	db = &caseDB{
		prefixTbl: make(map[string][]string),
		nameTbl:   make(map[string]string),
		cap:       cap,
		lock:      sync.RWMutex{},

		list: lrulist.NewLRUList(cap, func(item interface{}) error {
			tc := item.(*testcase.Testcase)

			for idx, item := range db.prefixTbl[tc.ID[:2]] {
				if item == tc.ID {
					db.prefixTbl[tc.ID[:2]][idx] = db.prefixTbl[tc.ID[:2]][len(db.prefixTbl[tc.ID[:2]])-1]
					db.prefixTbl[tc.ID[:2]] = db.prefixTbl[tc.ID[:2]][:len(db.prefixTbl[tc.ID[:2]])-1]
					break
				}
			}

			delete(db.nameTbl, tc.Name)

			return nil
		}),
	}
}

func GetCaseDB() *caseDB {
	return db
}

func (db *caseDB) Add(tc *testcase.Testcase) error {
	if exists, err := db.TestcaseExists(tc); exists {
		return err
	}

	db.lock.Lock()
	defer db.lock.Unlock()

	db.list.Set(tc.ID, tc)

	db.prefixTbl[tc.ID[:2]] = append(db.prefixTbl[tc.ID[:2]], tc.ID)
	db.nameTbl[tc.Name+":"+tc.Tag] = tc.ID

	return nil
}

func (db *caseDB) GetByID(id string) (*testcase.Testcase, error) {
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

	return rslt.(*testcase.Testcase), nil
}

func (db *caseDB) GetByNameTag(name string, tag string) (*testcase.Testcase, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	id, exists := db.nameTbl[name+":"+tag]
	if !exists {
		return nil, fmt.Errorf("%s:%s no such testcase", name, tag)
	}

	rslt, err := db.list.Get(id)
	// Fatal BUG occurs
	if err != nil {
		return nil, err
	}

	return rslt.(*testcase.Testcase), nil
}

func (db *caseDB) GetByPrefix(prefix string) (*testcase.Testcase, error) {
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
				return nil, fmt.Errorf("multiple testcases found with provided prefix: %q", prefix)
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

		return rslt.(*testcase.Testcase), nil
	}

	return nil, fmt.Errorf("%q: no such testcase", prefix)
}

func (db *caseDB) IsExistingID(id string) bool {
	db.lock.RLock()
	defer db.lock.RUnlock()

	for _, item := range db.prefixTbl[id[:2]] {
		if item == id {
			return true
		}
	}

	return false
}

func (db *caseDB) IsExistingNameTag(name string, tag string) bool {
	db.lock.RLock()
	defer db.lock.RUnlock()

	_, exists := db.nameTbl[name+":"+tag]

	return exists
}

func (db *caseDB) List() []*testcase.Testcase {
	tcs := []*testcase.Testcase{}

	db.lock.RLock()
	defer db.lock.RUnlock()

	db.list.Traverse(func(item interface{}) error {
		tc := item.(*testcase.Testcase)
		tcs = append(tcs, tc)

		return nil
	})

	return tcs
}

func (db *caseDB) TestcaseExists(tc *testcase.Testcase) (bool, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	for _, item := range db.prefixTbl[tc.ID[:2]] {
		if item == tc.ID {
			return true, fmt.Errorf("id: %q already exists", tc.ID)
		}
	}

	if _, exists := db.nameTbl[tc.Name+":"+tc.Tag]; exists {
		return true, fmt.Errorf("%s:%s already exists", tc.Name, tc.Tag)
	}

	return false, nil
}
