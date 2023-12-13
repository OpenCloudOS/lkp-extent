package daemon // import "github.com/AntiBargu/lkp-extent/lkp-master/daemon"

import (
	"context"
	"fmt"

	"go.uber.org/zap"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/job"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/task"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
)

func (d *Daemon) TaskInspect(ctx context.Context, req *cligrpc.TaskInspectRequest) (*cligrpc.TaskInspectResponse, error) {
	resp := &cligrpc.TaskInspectResponse{}

	d.Logger.Debug("task inspect request", zap.String("ID", req.ID))

	t, err := d.tasks.GetByPrefix(req.ID)
	if err != nil {
		d.Logger.Info("Unable to get task", zap.String("ID", req.ID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.ID)
	}

	resp.Info = &cligrpc.TaskMetadata{
		ID:    t.ID,
		Stat:  cligrpc.TaskMetadata_Status(t.Stat),
		CTime: t.CTime.Format(d.Cfg.Feature.Timestamp.Format),
		// FTime:       t.FTime.Format(d.Cfg.Feature.Timestamp.Format),
		Role:        cligrpc.TaskMetadata_RoleType(t.Role),
		JobID:       t.JobID,
		ResltPath:   t.RsltPath,
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
		resp.Info.FTime = t.FTime.Format(d.Cfg.Feature.Timestamp.Format)
	}

	return resp, nil
}

func (d *Daemon) TaskList(ctx context.Context, req *cligrpc.TaskListRequest) (*cligrpc.TaskListResponse, error) {
	resp := &cligrpc.TaskListResponse{
		InfoList: []*cligrpc.TaskMetadata{},
	}

	d.Logger.Debug("case list request", zap.String("Job ID", req.JobID))

	var j *job.Job
	var err error

	j, err = d.jobs.GetByPrefix(req.JobID)
	if err != nil {
		j, err = d.jobs.GetByName(req.JobID)
	}
	if err != nil {
		d.Logger.Info("Unable to get job", zap.String("ID", req.JobID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.JobID)
	}

	for _, t := range j.Tasks {
		item := &cligrpc.TaskMetadata{
			ID:    t.ID,
			Stat:  cligrpc.TaskMetadata_Status(t.Stat),
			CTime: t.CTime.Format(d.Cfg.Feature.Timestamp.Format),
			// FTime:       t.FTime.Format(d.Cfg.Feature.Timestamp.Format),
			Role:        cligrpc.TaskMetadata_RoleType(t.Role),
			JobID:       t.JobID,
			ResltPath:   t.RsltPath,
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

		resp.InfoList = append(resp.InfoList, item)
	}

	return resp, nil
}

func (d *Daemon) TaskResult(ctx context.Context, req *cligrpc.TaskResultRequest) (*cligrpc.TaskResultResponse, error) {
	resp := &cligrpc.TaskResultResponse{}

	d.Logger.Debug("task result request", zap.String("ID", req.ID))

	t, err := d.tasks.GetByPrefix(req.ID)
	if err != nil {
		d.Logger.Info("Unable to get task", zap.String("ID", req.ID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.ID)
	}

	if t.Stat != task.FINISH {
		d.Logger.Info("The task is not finished yet", zap.String("ID", req.ID), zap.Error(err))
		return resp, status.Errorf(codes.Unavailable, "%q doesn't exist", req.ID)
	}

	resp.File = fmt.Sprintf(taskPath+"%s/rslt.tar.gz", t.ID)

	return resp, nil
}
