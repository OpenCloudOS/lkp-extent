package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"context"
	"fmt"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	cli "github.com/urfave/cli/v3"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func NodeList(ctx *cli.Context) error {
	req := &cligrpc.NodeListRequest{}

	con, err := grpc.Dial("unix://"+cfg.Service.CliDaemon.Sock,
		grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		return grpcErrorParse(err)
	}
	defer con.Close()

	cc := cligrpc.NewCliClient(con)
	resp, err := cc.NodeList(context.Background(), req)
	if err != nil {
		return grpcErrorParse(err)
	}

	// t.b.d: install print config

	for _, header := range nodeListFmt {
		if header.show {
			fmt.Printf(header.fmt, header.discription)
		}
	}
	fmt.Println()

	for _, node := range resp.InfoList {
		item := []interface{}{
			node.ID[:12],   // 0
			node.IP,        // 1
			node.Port,      // 2
			node.KernelVer, // 3
			node.DistVer,   // 4
			node.Arch,      // 5
			node.FreeMem,   // 6
			node.DiskUsage, // 7
		}
		for i := range item {
			if nodeListFmt[i].show {
				fmt.Printf(nodeListFmt[i].fmt, item[i])
			}
		}
		fmt.Println()
	}

	return nil
}
