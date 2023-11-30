package monitor // import "github.com/AntiBargu/lkp-extent/lkp-master/monitor"

import (
	"context"
	"net"
	"time"

	"go.uber.org/zap"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/keepalive"
	"google.golang.org/grpc/peer"
	"google.golang.org/grpc/status"

	"github.com/AntiBargu/lkp-extent/lkp-master/pkg/stringid"
	"github.com/AntiBargu/lkp-extent/lkp-master/resource/node"

	mgrpc "github.com/AntiBargu/lkp-extent/api/monitor"
	schedgrpc "github.com/AntiBargu/lkp-extent/api/sched"
)

func (m *Monitor) Connect(ctx context.Context, req *mgrpc.ConnectRequest) (*mgrpc.ConnectResponse, error) {
	resp := &mgrpc.ConnectResponse{}

	peer, ok := peer.FromContext(ctx)
	if !ok {
		m.Logger.Info("Unable to get peer information associated with gRPC")
		return resp, status.Errorf(codes.PermissionDenied, "Unable to get peer information associated with gRPC")
	}

	ip, port, err := net.SplitHostPort(peer.Addr.String())
	if err != nil {
		m.Logger.Info("Unable to split host and port", zap.Error(err))
		return resp, status.Errorf(codes.InvalidArgument, "Unable to split host and port: %v", err)
	}

	if req.SchedPort == "" {
		return resp, status.Errorf(codes.InvalidArgument, "Sched service port is not set")
	}

	con, err := grpc.Dial(ip+":"+req.SchedPort,
		grpc.WithTransportCredentials(insecure.NewCredentials()),
		grpc.WithKeepaliveParams(keepalive.ClientParameters{
			Time:    time.Duration(m.Cfg.Service.Monitor.Keepalive) * time.Second,
			Timeout: time.Duration(m.Cfg.Service.Monitor.KeepaliveTimeout) * time.Second,
		}))
	if err != nil {
		return resp, status.Errorf(codes.PermissionDenied, "Failed to connect to Sched server: %v", err)
	}

	m.Logger.Debug("New connection established", zap.String("IP", ip), zap.String("Port", port))

	item := &node.Node{
		IP:          ip,
		Port:        port,
		KernelVer:   req.KernelVer,
		DistrVer:    req.DistVer,
		Arch:        req.Arch,
		FreeMem:     req.Stat.FreeMem,
		DiskUsage:   req.Stat.DiskUsage,
		SchedPort:   req.SchedPort,
		LoginTime:   time.Now(),
		SchedClient: schedgrpc.NewSchedClient(con),
	}

	for {
		item.ID = stringid.GenerateRandomID()
		if err := m.Nodes.Add(item.ID, item); err == nil {
			break
		}
	}

	resp.ID = item.ID

	return resp, nil
}

func (m *Monitor) StatUpdate(ctx context.Context, req *mgrpc.StatUpdateRequest) (*mgrpc.StatUpdateResponse, error) {
	return &mgrpc.StatUpdateResponse{}, nil
}

func (m *Monitor) Disconnect(ctx context.Context, req *mgrpc.DisconnectRequest) (*mgrpc.DisconnectResponse, error) {
	resp := &mgrpc.DisconnectResponse{}

	m.Logger.Debug("Disconnecting", zap.String("ID", req.ID))

	if err := m.Nodes.DeleteByID(req.ID); err != nil {
		m.Logger.Info("Unable to get node", zap.String("ID", req.ID), zap.Error(err))
		return resp, status.Errorf(codes.NotFound, "%q doesn't exist", req.ID)
	}

	return resp, nil
}
