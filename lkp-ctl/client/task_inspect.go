package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"context"
	"encoding/json"
	"fmt"
	"os"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	cli "github.com/urfave/cli/v3"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	yaml "gopkg.in/yaml.v2"
)

func TaskInspect(ctx *cli.Context) error {
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
	for _, id := range ctx.Args().Slice() {
		resp, err := cc.TaskInspect(context.Background(), &cligrpc.TaskInspectRequest{ID: id})
		if err != nil {
			fmt.Fprintf(os.Stderr, "%v\n", grpcErrorParse(err))
			continue
		}

		var rslt []byte
		if ctx.Bool("yaml") {
			rslt, err = yaml.Marshal(resp.Info)
		} else {
			rslt, err = json.MarshalIndent(resp.Info, "", "    ")

		}
		if err != nil {
			return ErrDeserialization
		}
		fmt.Println(string(rslt))
	}

	return nil
}
