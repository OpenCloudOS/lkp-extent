package task // import "github.com/AntiBargu/lkp-extent/lkp-node/task"

const (
	HOST = iota + 1
	CONTAINER
)

type Task struct {
	ID       string
	Role     int32
	CaseID   string
	CaseName string
}
