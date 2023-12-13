package nodedb // import "github.com/AntiBargu/lkp-extent/lkp-master/pkg/nodedb"

import (
	"errors"
	"strings"
	"sync"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/node"
)

type nodeDB struct {
	dict map[string][]*node.Node
	size int
	lock sync.RWMutex
}

var db *nodeDB
var once sync.Once

func GetNodeDB() *nodeDB {
	once.Do(func() {
		db = &nodeDB{
			dict: make(map[string][]*node.Node),
			size: 0,
			lock: sync.RWMutex{},
		}
	})

	return db
}

func (db *nodeDB) Add(id string, node *node.Node) error {
	if id == "" {
		return errors.New("id can't be empty")
	}

	if len(id) < 2 {
		return errors.New("id is too short")
	}

	db.lock.RLock()
	for _, item := range db.dict[id[:2]] {
		if item.ID == id {
			return errors.New("id already exists")
		}
	}
	db.lock.RUnlock()

	db.lock.Lock()
	defer db.lock.Unlock()

	db.dict[id[:2]] = append(db.dict[id[:2]], node)
	db.size += 1

	return nil
}

func (db *nodeDB) GetByID(id string) (*node.Node, error) {
	if id == "" {
		return nil, errors.New("id can't be empty")
	}

	if len(id) < 2 {
		return nil, errors.New("id is too short")
	}

	db.lock.RLock()
	defer db.lock.RUnlock()

	for _, item := range db.dict[id[:2]] {
		if item.ID == id {
			return item, nil
		}
	}

	return nil, errors.New("no such node")
}

func (db *nodeDB) GetByPrefix(prefix string) (*node.Node, error) {
	if prefix == "" {
		return nil, errors.New("prefix can't be empty")
	}

	if len(prefix) < 2 {
		return nil, errors.New("prefix is too short")
	}

	var node *node.Node

	db.lock.RLock()
	defer db.lock.RUnlock()

	for _, item := range db.dict[prefix[:2]] {
		if strings.HasPrefix(item.ID, prefix) {
			if node != nil {
				return nil, errors.New("multiple nodes found with provided prefix: " + prefix)
			}
			node = item
		}
	}

	if node != nil {
		return node, nil
	}

	return nil, errors.New("no such node")
}

func (db *nodeDB) DeleteByID(id string) error {
	if id == "" {
		return errors.New("id can't be empty")
	}

	if len(id) < 2 {
		return errors.New("id is too short")
	}

	db.lock.Lock()
	defer db.lock.Unlock()

	for idx, item := range db.dict[id[:2]] {
		if item.ID == id {
			db.dict[id[:2]][idx] = db.dict[id[:2]][len(db.dict[id[:2]])-1]
			db.dict[id[:2]] = db.dict[id[:2]][:len(db.dict[id[:2]])-1]
			db.size -= 1
			return nil
		}
	}

	return errors.New("no such node")
}

func (db *nodeDB) List() []*node.Node {
	nodes := []*node.Node{}

	db.lock.RLock()
	defer db.lock.RUnlock()

	for _, sublist := range db.dict {
		nodes = append(nodes, sublist...)
	}

	return nodes
}

func (db *nodeDB) Size() int {
	return db.size
}
