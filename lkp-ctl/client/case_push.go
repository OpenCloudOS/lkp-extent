package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"context"
	"fmt"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	cli "github.com/urfave/cli/v3"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func CasePush(ctx *cli.Context) error {
	if ctx.Args().Len() < 1 {
		return ErrAtLeastOneArgument
	}

	con, err := grpc.Dial("unix://"+cfg.Service.CliDaemon.Sock,
		grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		return grpcErrorParse(err)
	}
	defer con.Close()

	cc := cligrpc.NewCliClient(con)

	if ctx.Bool("all") {
		req := &cligrpc.CasePushToAllRequest{}

		req.CaseID = ctx.Args().First()

		_, err := cc.CasePushToAll(context.Background(), req)
		if err != nil {
			fmt.Fprintln(ctx.Command.ErrWriter, grpcErrorParse(err))
		}
	} else {
		req := &cligrpc.CasePushRequest{}

		if ctx.Args().Len() < 2 {
			return ErrAtLeastTwoArguments
		}

		req.CaseID = ctx.Args().First()
		req.NodeIDs = append(req.NodeIDs, ctx.Args().Tail()...)

		resp, err := cc.CasePush(context.Background(), req)
		if err != nil {
			fmt.Fprintln(ctx.Command.ErrWriter, grpcErrorParse(err))
		} else {
			for _, rslt := range resp.Rslts {
				fmt.Println(rslt)
			}
		}
	}

	return nil
}
