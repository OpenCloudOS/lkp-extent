package daemon // import "github.com/AntiBargu/lkp-extent/lkp-master/daemon"

import (
	"context"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
)

func (d *Daemon) Init(ctx context.Context, req *cligrpc.InitRequest) (*cligrpc.InitResponse, error) {
	resp := &cligrpc.InitResponse{}

	d.Logger.Debug("init request")

	return resp, nil
}
