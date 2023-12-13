package node // import "github.com/AntiBargu/lkp-extent/lkp-master/resource/node"

import (
	"time"

	schedgrpc "github.com/AntiBargu/lkp-extent/api/sched"
)

type Node struct {
	ID          string
	IP          string
	Port        string
	KernelVer   string
	DistrVer    string
	Arch        string
	FreeMem     string
	DiskUsage   string
	SchedPort   string
	LoginTime   time.Time
	SchedClient schedgrpc.SchedClient
}

type Store interface {
	// Add appends a new Node to the store.
	Add(string, *Node) error

	// DeleteByID removes a Node from the store by the ID it was stored with.
	DeleteByID(string) error

	// GetByID returns a Node from the store by the ID it was stored with.
	GetByID(string) (*Node, error)
	// GetByName returns a Node from the store by Name it was stored with.
	// GetByName(string) (*Node, error)
	// GetByPrefix returns a Node from the store by the ID Prefix it was stored with.
	GetByPrefix(string) (*Node, error)

	// List returns a list of Nodes from the store.
	List() []*Node

	// Size returns the number of Nodes in the store.
	Size() int
}
