package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	cli "github.com/urfave/cli/v3"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func CaseUpdate(ctx *cli.Context) error {
	req := &cligrpc.CaseUpdateRequest{}

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
	for _, item := range ctx.Args().Slice() {
		path, tag := nameTagParse(item)

		if !strings.HasSuffix(strings.ToLower(path), ".yaml") {
			fmt.Fprintf(ctx.Command.ErrWriter, "%s: %s\n", path, ErrCaseFormat)
			continue
		}

		data, err := os.ReadFile(path)
		if err != nil {
			fmt.Fprintln(ctx.Command.ErrWriter, err)
			continue
		}

		req.Name, req.Tag = strings.Split(filepath.Base(path), ".")[0], tag
		req.File = data
		_, err = cc.CaseUpdate(context.Background(), req)
		if err != nil {
			fmt.Fprintln(ctx.Command.ErrWriter, grpcErrorParse(err))
		}
	}

	return nil
}
