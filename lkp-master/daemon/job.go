package daemon // import "github.com/AntiBargu/lkp-extent/lkp-master/daemon"

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"go.uber.org/zap"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/job"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/node"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/task"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/testcase"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	schedgrpc "github.com/AntiBargu/lkp-extent/api/sched"
)

func (d *Daemon) JobInspect(ctx context.Context, req *cligrpc.JobInspectRequest) (*cligrpc.JobInspectResponse, error) {
	resp := &cligrpc.JobInspectResponse{}

	d.Logger.Debug("job inspect request", zap.String("Job ID", req.ID))

	var j *job.Job
	var err error

	j, err = d.jobs.GetByPrefix(req.ID)
	if err != nil {
		j, err = d.jobs.GetByName(req.ID)
	}
	if err != nil {
		d.Logger.Info("Unable to get job", zap.String("ID", req.ID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.ID)
	}

	tasks := []*cligrpc.TaskMetadata{}
	for _, t := range j.Tasks {
		item := &cligrpc.TaskMetadata{
			ID:    t.ID,
			Stat:  cligrpc.TaskMetadata_Status(t.Stat),
			CTime: t.CTime.Format(d.Cfg.Feature.Timestamp.Format),
			// FTime:       t.FTime.Format(d.Cfg.Feature.Timestamp.Format),
			Role:        cligrpc.TaskMetadata_RoleType(t.Role),
			ContainerID: t.ContainerID,
			CaseInfo: &cligrpc.CaseMetadata{
				ID:    t.Tc.ID,
				Name:  t.Tc.Name,
				Tag:   t.Tc.Tag,
				Path:  t.Tc.Path,
				CTime: t.Tc.CTime.Format(d.Cfg.Feature.Timestamp.Format),
				ATime: t.Tc.ATime.Format(d.Cfg.Feature.Timestamp.Format),
				MTime: t.Tc.MTime.Format(d.Cfg.Feature.Timestamp.Format),
				Size:  t.Tc.Size,
			},
			NodeInfo: &cligrpc.NodeMetadata{
				ID:        t.Node.ID,
				IP:        t.Node.IP,
				Port:      t.Node.Port,
				KernelVer: t.Node.KernelVer,
				DistVer:   t.Node.DistrVer,
				Arch:      t.Node.Arch,
				FreeMem:   t.Node.FreeMem,
				DiskUsage: t.Node.DiskUsage,
			},
		}
		if t.Stat == task.FINISH {
			item.FTime = t.FTime.Format(d.Cfg.Feature.Timestamp.Format)
		}

		tasks = append(tasks, item)
	}

	resp.Info = &cligrpc.JobMetadata{
		ID:        j.ID,
		Name:      j.Name,
		Stat:      cligrpc.JobMetadata_Status(j.Stat),
		FinishCnt: j.FCnt,
		CTime:     j.CTime.Format(d.Cfg.Feature.Timestamp.Format),
		// FTime: j.FTime.Format(d.Cfg.Feature.Timestamp.Format),
		CaseInfo: &cligrpc.CaseMetadata{
			ID:    j.Tc.ID,
			Name:  j.Tc.Name,
			Tag:   j.Tc.Tag,
			Path:  j.Tc.Path,
			CTime: j.Tc.CTime.Format(d.Cfg.Feature.Timestamp.Format),
			ATime: j.Tc.ATime.Format(d.Cfg.Feature.Timestamp.Format),
			MTime: j.Tc.MTime.Format(d.Cfg.Feature.Timestamp.Format),
			Size:  j.Tc.Size,
		},
		Tasks: tasks,
	}
	if j.Stat == job.FINISH {
		resp.Info.FTime = j.FTime.Format(d.Cfg.Feature.Timestamp.Format)
	}

	return resp, nil
}

func (d *Daemon) JobList(ctx context.Context, req *cligrpc.JobListRequest) (*cligrpc.JobListResponse, error) {
	resp := &cligrpc.JobListResponse{
		InfoList: []*cligrpc.JobMetadata{},
	}

	d.Logger.Debug("job list request")

	for _, item := range d.jobs.List() {
		j := &cligrpc.JobMetadata{
			ID:        item.ID,
			Name:      item.Name,
			Stat:      cligrpc.JobMetadata_Status(item.Stat),
			FinishCnt: item.FCnt,
			CTime:     item.CTime.Format(d.Cfg.Feature.Timestamp.Format),
			// FTime: item.FTime.Format(d.Cfg.Feature.Timestamp.Format),
			CaseInfo: &cligrpc.CaseMetadata{
				ID:    item.Tc.ID,
				Name:  item.Tc.Name,
				Tag:   item.Tc.Tag,
				Path:  item.Tc.Path,
				CTime: item.Tc.CTime.Format(d.Cfg.Feature.Timestamp.Format),
				ATime: item.Tc.ATime.Format(d.Cfg.Feature.Timestamp.Format),
				MTime: item.Tc.MTime.Format(d.Cfg.Feature.Timestamp.Format),
				Size:  item.Tc.Size,
			},
			Tasks: []*cligrpc.TaskMetadata{},
		}

		if item.Stat == job.FINISH {
			j.FTime = item.FTime.Format(d.Cfg.Feature.Timestamp.Format)
		}

		for _, t := range item.Tasks {
			it := &cligrpc.TaskMetadata{
				ID:    t.ID,
				Stat:  cligrpc.TaskMetadata_Status(t.Stat),
				CTime: t.CTime.Format(d.Cfg.Feature.Timestamp.Format),
				// FTime:       t.FTime.Format(d.Cfg.Feature.Timestamp.Format),
				Role:        cligrpc.TaskMetadata_RoleType(t.Role),
				ContainerID: t.ContainerID,
				CaseInfo: &cligrpc.CaseMetadata{
					ID:    t.Tc.ID,
					Name:  t.Tc.Name,
					Tag:   t.Tc.Tag,
					Path:  t.Tc.Path,
					CTime: t.Tc.CTime.Format(d.Cfg.Feature.Timestamp.Format),
					ATime: t.Tc.ATime.Format(d.Cfg.Feature.Timestamp.Format),
					MTime: t.Tc.MTime.Format(d.Cfg.Feature.Timestamp.Format),
					Size:  t.Tc.Size,
				},
				NodeInfo: &cligrpc.NodeMetadata{
					ID:        t.Node.ID,
					IP:        t.Node.IP,
					Port:      t.Node.Port,
					KernelVer: t.Node.KernelVer,
					DistVer:   t.Node.DistrVer,
					Arch:      t.Node.Arch,
					FreeMem:   t.Node.FreeMem,
					DiskUsage: t.Node.DiskUsage,
				},
			}
			if t.Stat == task.FINISH {
				it.FTime = t.FTime.Format(d.Cfg.Feature.Timestamp.Format)
			}

			j.Tasks = append(j.Tasks, it)
		}

		resp.InfoList = append(resp.InfoList, j)
	}

	return resp, nil

}

func (d *Daemon) JobRun(ctx context.Context, req *cligrpc.JobRunRequest) (*cligrpc.JobRunResponse, error) {
	resp := &cligrpc.JobRunResponse{
		Rslts: []string{},
	}

	d.Logger.Debug("job run request", zap.String("Case ID", req.CaseID), zap.String("Job Name", req.Name),
		zap.Strings("Node ID List", req.NodeIDs))

	var tc *testcase.Testcase
	var err error

	parts := strings.Split(req.CaseID, ":")
	if len(parts) == 2 {
		tc, err = d.testcases.GetByNameTag(parts[0], parts[1])
	} else {
		tc, err = d.testcases.GetByPrefix(req.CaseID)
		if err != nil {
			tc, err = d.testcases.GetByNameTag(req.CaseID, "latest")
		}
	}

	if err != nil {
		d.Logger.Info("Unable to get testcase", zap.String("Case ID", req.CaseID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.CaseID)
	}

	if d.jobs.IsExistingName(req.Name) {
		d.Logger.Info("Name already exists", zap.String("Job Name", req.Name))
		return resp, status.Errorf(codes.AlreadyExists, "%q already exists", req.Name)
	}

	var j *job.Job
	for {
		j = job.NewJob(req.Name)
		if !d.jobs.IsExistingID(j.ID) && !d.jobs.IsExistingName(j.Name) {
			j.Tc = tc
			break
		}
	}

	msgCh := make(chan string, len(req.NodeIDs))
	tasksCh := make(chan []*task.Task, len(req.NodeIDs))
	wg := sync.WaitGroup{}

	for _, nodeID := range req.NodeIDs {
		wg.Add(1)
		go func(nodeID string) {
			defer wg.Done()

			sreq := &schedgrpc.JobRunRequest{
				CaseID:   tc.ID,
				CaseName: tc.Name,
				Tasks:    []*schedgrpc.TaskMetaData{},
			}

			node, err := d.nodes.GetByPrefix(nodeID)
			if err != nil {
				d.Logger.Info("Unable to get node", zap.String("Node ID", nodeID))
				msgCh <- fmt.Sprintf("%q: doesn't exist", nodeID)
				return
			}

			tasks := []*task.Task{}
			for {
				t := task.NewTask()
				if !d.tasks.IsExistingID(t.ID) {
					t.JobID = j.ID
					t.Role = task.HOST
					t.Node = node
					t.Tc = tc
					tasks = append(tasks, t)
					sreq.Tasks = append(sreq.Tasks, &schedgrpc.TaskMetaData{
						ID:   t.ID,
						Role: schedgrpc.TaskMetaData_RoleType(t.Role),
					})
					break
				}
			}
			var i int64
			for ; i < req.ContainerCnt; i++ {
				for {
					t := task.NewTask()
					if !d.tasks.IsExistingID(t.ID) {
						t.JobID = j.ID
						t.Role = task.CONTAINER
						t.Node = node
						t.Tc = tc
						tasks = append(tasks, t)
						sreq.Tasks = append(sreq.Tasks, &schedgrpc.TaskMetaData{
							ID:   t.ID,
							Role: schedgrpc.TaskMetaData_RoleType(t.Role),
						})
						break
					}
				}
			}

			d.Logger.Debug("Distribute tasks to nodes", zap.String("Node ID", node.ID))
			_, err = node.SchedClient.JobRun(context.Background(), sreq)
			if err != nil {
				d.Logger.Info("Distribute tasks failed", zap.String("Node ID", node.ID))
				msgCh <- fmt.Sprintf("distribute tasks to %s failed: %v", node.ID, err)
				for _, t := range tasks {
					t.Stat = task.ERROR
					t.FTime = time.Now()
				}
			}
			tasksCh <- tasks
		}(nodeID)
	}

	wg.Wait()

	close(msgCh)
	close(tasksCh)

	for msg := range msgCh {
		resp.Rslts = append(resp.Rslts, msg)
	}

	for tasks := range tasksCh {
		for _, t := range tasks {
			if t.Stat == task.STANDBY {
				t.Stat = task.RUNNING
			} else {
				j.FCnt += 1
			}

			// Add task to the job's task list
			j.Tasks = append(j.Tasks, t)

			d.Logger.Debug("Add task to DB", zap.String("Task ID", t.ID))
			if err := d.tasks.Add(t); err != nil {
				// Maybe need to cancel the job.
				d.Logger.Info("Task add failed", zap.Error(err))
				return resp, status.Errorf(codes.Unavailable, "Task add failed: %v", err)
			}
		}
	}

	d.Logger.Debug("Add job to DB", zap.String("Job ID", j.ID))
	if j.FCnt < int64(len(j.Tasks)) {
		j.Stat = job.RUNNING
	} else {
		j.Stat = job.FINISH
	}

	if err := d.jobs.Add(j); err != nil {
		// Maybe need to cancel the job.
		d.Logger.Info("Job add failed", zap.Error(err))
		return resp, status.Errorf(codes.Unavailable, "Job add failed: %v", err)
	}

	return resp, nil
}

func (d *Daemon) JobRunToAll(ctx context.Context, req *cligrpc.JobRunToAllRequest) (*cligrpc.JobRunToAllResponse, error) {
	resp := &cligrpc.JobRunToAllResponse{
		Rslts: []string{},
	}

	d.Logger.Debug("job run request", zap.String("Case ID", req.CaseID), zap.String("Job Name", req.Name))

	if d.nodes.Size() == 0 {
		d.Logger.Info("Nodes don't exist")
		return resp, status.Errorf(codes.NotFound, "Nodes don't exist")
	}

	var tc *testcase.Testcase
	var err error

	parts := strings.Split(req.CaseID, ":")
	if len(parts) == 2 {
		tc, err = d.testcases.GetByNameTag(parts[0], parts[1])
	} else {
		tc, err = d.testcases.GetByPrefix(req.CaseID)
		if err != nil {
			tc, err = d.testcases.GetByNameTag(req.CaseID, "latest")
		}
	}

	if err != nil {
		d.Logger.Info("Unable to get testcase", zap.String("Case ID", req.CaseID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.CaseID)
	}

	if d.jobs.IsExistingName(req.Name) {
		d.Logger.Info("Name already exists", zap.String("Job Name", req.Name))
		return resp, status.Errorf(codes.AlreadyExists, "%q already exists", req.Name)
	}

	var j *job.Job
	for {
		j = job.NewJob(req.Name)
		if !d.jobs.IsExistingID(j.ID) && !d.jobs.IsExistingName(j.Name) {
			j.Tc = tc
			break
		}
	}

	msgCh := make(chan string, d.nodes.Size())
	tasksCh := make(chan []*task.Task, d.nodes.Size())
	wg := sync.WaitGroup{}

	for _, item := range d.nodes.List() {
		wg.Add(1)
		go func(n *node.Node) {
			defer wg.Done()

			sreq := &schedgrpc.JobRunRequest{
				CaseID:   tc.ID,
				CaseName: tc.Name,
				Tasks:    []*schedgrpc.TaskMetaData{},
			}

			tasks := []*task.Task{}
			for {
				t := task.NewTask()
				if !d.tasks.IsExistingID(t.ID) {
					t.JobID = j.ID
					t.Role = task.HOST
					t.Node = n
					t.Tc = tc
					tasks = append(tasks, t)
					sreq.Tasks = append(sreq.Tasks, &schedgrpc.TaskMetaData{
						ID:   t.ID,
						Role: schedgrpc.TaskMetaData_RoleType(t.Role),
					})
					break
				}
			}
			var i int64
			for ; i < req.ContainerCnt; i++ {
				for {
					t := task.NewTask()
					if !d.tasks.IsExistingID(t.ID) {
						t.JobID = j.ID
						t.Role = task.CONTAINER
						t.Node = n
						t.Tc = tc
						tasks = append(tasks, t)
						sreq.Tasks = append(sreq.Tasks, &schedgrpc.TaskMetaData{
							ID:   t.ID,
							Role: schedgrpc.TaskMetaData_RoleType(t.Role),
						})
						break
					}
				}
			}
			tasksCh <- tasks

			d.Logger.Debug("Distribute tasks to nodes", zap.String("Node ID", n.ID))
			_, err = n.SchedClient.JobRun(context.Background(), sreq)
			if err != nil {
				d.Logger.Info("Distribute tasks failed", zap.String("Node ID", n.ID))
				msgCh <- fmt.Sprintf("distribute tasks to %s failed: %v", n.ID, err)
			}
		}(item)
	}

	wg.Wait()

	close(msgCh)
	close(tasksCh)

	for msg := range msgCh {
		resp.Rslts = append(resp.Rslts, msg)
	}

	for tasks := range tasksCh {
		for _, t := range tasks {
			t.Stat = task.RUNNING

			// Add task to the job's task list
			j.Tasks = append(j.Tasks, t)

			d.Logger.Debug("Add task to DB", zap.String("Task ID", t.ID))
			if err := d.tasks.Add(t); err != nil {
				// Maybe need to cancel the job.
				d.Logger.Info("Task add failed", zap.Error(err))
				return resp, status.Errorf(codes.Unavailable, "Task add failed: %v", err)
			}
		}
	}

	d.Logger.Debug("Add job to DB", zap.String("Job ID", j.ID))
	j.Stat = job.RUNNING
	if err := d.jobs.Add(j); err != nil {
		// Maybe need to cancel the job.
		d.Logger.Info("Job add failed", zap.Error(err))
		return resp, status.Errorf(codes.Unavailable, "Job add failed: %v", err)
	}

	return resp, nil
}
