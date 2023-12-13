package taskqueue // import "github.com/AntiBargu/lkp-extent/lkp-node/pkg/taskqueue"

import (
	"sync"
)

type TaskQueue struct {
	queue []interface{}
	cond  *sync.Cond
}

func NewTaskQueue() *TaskQueue {
	return &TaskQueue{
		queue: []interface{}{},
		cond:  sync.NewCond(&sync.Mutex{}),
	}
}

func (tq *TaskQueue) EnQueue(item interface{}) {
	tq.cond.L.Lock()
	defer tq.cond.L.Unlock()

	tq.queue = append(tq.queue, item)
	tq.cond.Broadcast()
}

func (tq *TaskQueue) DeQueue() interface{} {
	tq.cond.L.Lock()
	defer tq.cond.L.Unlock()

	if len(tq.queue) == 0 {
		tq.cond.Wait()
	}

	rslt := tq.queue[0]
	tq.queue = tq.queue[1:]

	return rslt
}
