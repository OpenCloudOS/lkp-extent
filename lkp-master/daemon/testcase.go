package daemon // import "github.com/AntiBargu/lkp-extent/lkp-master/daemon"

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"go.uber.org/zap"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/AntiBargu/lkp-extent/lkp-master/resource/node"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/testcase"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	schedgrpc "github.com/AntiBargu/lkp-extent/api/sched"
)

func (d *Daemon) CaseInspect(ctx context.Context, req *cligrpc.CaseInspectRequest) (*cligrpc.CaseInspectResponse, error) {
	resp := &cligrpc.CaseInspectResponse{}

	d.Logger.Debug("case inspect request", zap.String("ID", req.ID))

	var tc *testcase.Testcase
	var err error

	parts := strings.Split(req.ID, ":")
	if len(parts) == 2 {
		tc, err = d.testcases.GetByNameTag(parts[0], parts[1])
	} else {
		tc, err = d.testcases.GetByPrefix(req.ID)
		if err != nil {
			tc, err = d.testcases.GetByNameTag(req.ID, "latest")
		}
	}

	if err != nil {
		d.Logger.Info("Unable to get testcase", zap.String("ID", req.ID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.ID)
	}

	tc.ATime = time.Now()
	resp.Info = &cligrpc.CaseMetadata{
		ID:    tc.ID,
		Name:  tc.Name,
		Tag:   tc.Tag,
		Path:  tc.Path,
		CTime: tc.CTime.Format(d.Cfg.Feature.Timestamp.Format),
		ATime: tc.ATime.Format(d.Cfg.Feature.Timestamp.Format),
		MTime: tc.MTime.Format(d.Cfg.Feature.Timestamp.Format),
		Size:  tc.Size,
	}

	return resp, nil
}

func (d *Daemon) CaseList(ctx context.Context, req *cligrpc.CaseListRequest) (*cligrpc.CaseListResponse, error) {
	resp := &cligrpc.CaseListResponse{
		InfoList: []*cligrpc.CaseMetadata{},
	}

	d.Logger.Debug("case list request")

	for _, tc := range d.testcases.List() {
		resp.InfoList = append(resp.InfoList, &cligrpc.CaseMetadata{
			ID:    tc.ID,
			Name:  tc.Name,
			Tag:   tc.Tag,
			Path:  tc.Path,
			CTime: tc.CTime.Format(d.Cfg.Feature.Timestamp.Format),
			ATime: tc.ATime.Format(d.Cfg.Feature.Timestamp.Format),
			MTime: tc.MTime.Format(d.Cfg.Feature.Timestamp.Format),
			Size:  tc.Size,
		})
	}

	return resp, nil
}

func (d *Daemon) CaseAdd(ctx context.Context, req *cligrpc.CaseAddRequest) (*cligrpc.CaseAddResponse, error) {
	resp := &cligrpc.CaseAddResponse{}

	d.Logger.Debug("case add request", zap.String("Name", req.Name), zap.String("Tag", req.Tag))

	if d.testcases.IsExistingNameTag(req.Name, req.Tag) {
		d.Logger.Info("Name:Tag already exists", zap.String("Name", req.Name), zap.String("Tag", req.Tag))
		return resp, status.Errorf(codes.AlreadyExists, "%s:%s already exists", req.Name, req.Tag)
	}

	var tc *testcase.Testcase
	for {
		tc = testcase.NewTestcase()
		if !d.testcases.IsExistingID(tc.ID) {
			tc.Name = req.Name
			tc.Tag = req.Tag
			tc.Path = fmt.Sprintf(casePath+"%s/%s.yaml", tc.ID, tc.Name)
			tc.Size = uint64(len(req.File))
			break
		}
	}

	if err := os.MkdirAll(filepath.Dir(tc.Path), os.ModePerm); err != nil {
		d.Logger.Info("Create directory failed", zap.Error(err))
		return resp, status.Errorf(codes.Internal, "Create directory failed: %q: %v", filepath.Dir(tc.Path), err)
	}

	if err := os.WriteFile(tc.Path, req.File, 0644); err != nil {
		d.Logger.Info("Testcase write failed", zap.Error(err))
		return resp, status.Errorf(codes.Internal, "Testcase write failed: %q: %v", tc.Path, err)
	}

	d.Logger.Debug("Add testcase to DB", zap.String("Case ID", tc.ID), zap.String("Name", req.Name), zap.String("Tag", req.Tag))
	if err := d.testcases.Add(tc); err != nil {
		d.Logger.Info("Testcase add failed", zap.Error(err))
		return resp, status.Errorf(codes.Unavailable, "Testcase add failed: %v", err)
	}

	if err := os.WriteFile(tc.Path, req.File, 0644); err != nil {
		d.Logger.Info("Testcase write failed", zap.Error(err))
		return resp, status.Errorf(codes.Internal, "Testcase write failed: %q: %v", tc.Path, err)
	}

	return resp, nil
}

func (d *Daemon) CaseUpdate(ctx context.Context, req *cligrpc.CaseUpdateRequest) (*cligrpc.CaseUpdateResponse, error) {
	resp := &cligrpc.CaseUpdateResponse{}

	d.Logger.Debug("case update request", zap.String("Name", req.Name), zap.String("Tag", req.Tag))

	tc, err := d.testcases.GetByNameTag(req.Name, req.Tag)
	if err != nil {
		return resp, status.Errorf(codes.NotFound, "%s:%s doesn't exist", req.Name, req.Tag)
	}

	// Here is a concurrency safety issues
	tc.ATime = time.Now()
	tc.MTime = time.Now()
	tc.Size = uint64(len(req.File))
	if err := os.WriteFile(tc.Path, req.File, 0644); err != nil {
		d.Logger.Info("Testcase write failed", zap.Error(err))
		return resp, status.Errorf(codes.Internal, "Testcase write failed: %q: %v", tc.Path, err)
	}

	return resp, nil
}

func (d *Daemon) CasePush(ctx context.Context, req *cligrpc.CasePushRequest) (*cligrpc.CasePushResponse, error) {
	resp := &cligrpc.CasePushResponse{
		Rslts: []string{},
	}

	d.Logger.Debug("case push request", zap.String("Case ID", req.CaseID), zap.Strings("Node ID List", req.NodeIDs))

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

	tc.ATime = time.Now()
	data, err := os.ReadFile(tc.Path)
	if err != nil {
		d.Logger.Info("Testcase read failed", zap.Error(err))
		return resp, status.Errorf(codes.Internal, "Testcase read failed: %q: %v", tc.Path, err)
	}

	sreq := &schedgrpc.CasePushRequest{
		ID:   tc.ID,
		Name: tc.Name,
		File: data,
	}

	msgCh := make(chan string, len(req.NodeIDs))
	wg := sync.WaitGroup{}

	for _, item := range req.NodeIDs {
		wg.Add(1)

		go func(nodeID string) {
			defer wg.Done()

			node, err := d.nodes.GetByPrefix(nodeID)
			if err != nil {
				d.Logger.Info("Unable to get node", zap.String("Node ID", nodeID), zap.Error(err))
				msgCh <- fmt.Sprintf("%q: doesn't exist", nodeID)
				return
			}

			d.Logger.Debug("Push testcase to node", zap.String("Case ID", tc.ID), zap.String("Node ID", node.ID))
			_, err = node.SchedClient.CasePush(context.Background(), sreq)
			if err != nil {
				d.Logger.Info("Testcase push failed", zap.String("Node ID", node.ID), zap.String("Case ID", tc.ID), zap.Error(err))
				msgCh <- fmt.Sprintf("push %q to %q failed: %v", tc.ID, node.ID, err)
			}
		}(item)
	}

	wg.Wait()

	close(msgCh)

	for msg := range msgCh {
		resp.Rslts = append(resp.Rslts, msg)
	}

	return resp, nil
}

func (d *Daemon) CasePushToAll(ctx context.Context, req *cligrpc.CasePushToAllRequest) (*cligrpc.CasePushToAllResponse, error) {
	resp := &cligrpc.CasePushToAllResponse{
		Rslts: []string{},
	}

	d.Logger.Debug("case push to all request", zap.String("Case ID", req.CaseID))

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

	tc.ATime = time.Now()
	data, err := os.ReadFile(tc.Path)
	if err != nil {
		d.Logger.Info("Testcase read failed", zap.Error(err))
		return resp, status.Errorf(codes.Internal, "Testcase read failed: %q: %v", tc.Path, err)
	}

	sreq := &schedgrpc.CasePushRequest{
		ID:   tc.ID,
		Name: tc.Name,
		File: data,
	}

	msgCh := make(chan string, d.nodes.Size())
	wg := sync.WaitGroup{}

	for _, item := range d.nodes.List() {
		wg.Add(1)

		go func(node *node.Node) {
			defer wg.Done()

			d.Logger.Debug("Push testcase to node", zap.String("Case ID", tc.ID), zap.String("Node ID", node.ID))
			_, err = node.SchedClient.CasePush(context.Background(), sreq)
			if err != nil {
				d.Logger.Info("Testcase push failed", zap.String("Node ID", node.ID), zap.String("Case ID", tc.ID))
				msgCh <- fmt.Sprintf("push %q to %q failed: %v", tc.ID, node.ID, err)
			}
		}(item)
	}

	wg.Wait()

	close(msgCh)

	for msg := range msgCh {
		resp.Rslts = append(resp.Rslts, msg)
	}

	return resp, nil
}
