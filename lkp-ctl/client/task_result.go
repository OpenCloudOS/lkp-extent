package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"context"
	"fmt"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	cli "github.com/urfave/cli/v3"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func TaskResult(ctx *cli.Context) error {
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
	resp, err := cc.TaskResult(context.Background(), &cligrpc.TaskResultRequest{ID: ctx.Args().First()})
	if err != nil {
		return grpcErrorParse(err)
	}

	fmt.Println(resp.File)

	return nil
}
