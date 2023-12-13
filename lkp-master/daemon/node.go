package daemon // import "github.com/AntiBargu/lkp-extent/lkp-master/daemon"

import (
	"context"

	"go.uber.org/zap"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
)

func (d *Daemon) NodeInspect(ctx context.Context, req *cligrpc.NodeInspectRequest) (*cligrpc.NodeInspectResponse, error) {
	resp := &cligrpc.NodeInspectResponse{}

	d.Logger.Debug("node inspect request", zap.String("ID", req.ID))

	node, err := d.nodes.GetByPrefix(req.ID)
	if err != nil {
		d.Logger.Info("Unable to get node", zap.String("ID", req.ID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.ID)
	}

	resp.Info = &cligrpc.NodeMetadata{
		ID:        node.ID,
		IP:        node.IP,
		Port:      node.Port,
		KernelVer: node.KernelVer,
		DistVer:   node.DistrVer,
		Arch:      node.Arch,
		FreeMem:   node.FreeMem,
		DiskUsage: node.DiskUsage,
	}

	return resp, nil
}

func (d *Daemon) NodeList(ctx context.Context, req *cligrpc.NodeListRequest) (*cligrpc.NodeListResponse, error) {
	resp := &cligrpc.NodeListResponse{
		InfoList: []*cligrpc.NodeMetadata{},
	}

	d.Logger.Debug("node list request")

	for _, node := range d.nodes.List() {
		resp.InfoList = append(resp.InfoList,
			&cligrpc.NodeMetadata{
				ID:        node.ID,
				IP:        node.IP,
				Port:      node.Port,
				KernelVer: node.KernelVer,
				DistVer:   node.DistrVer,
				Arch:      node.Arch,
				FreeMem:   node.FreeMem,
				DiskUsage: node.DiskUsage,
			},
		)
	}

	return resp, nil
}
