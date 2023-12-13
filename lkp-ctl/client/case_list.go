package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"context"
	"fmt"
	"strconv"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	cli "github.com/urfave/cli/v3"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func CaseList(ctx *cli.Context) error {
	req := &cligrpc.CaseListRequest{}

	con, err := grpc.Dial("unix://"+cfg.Service.CliDaemon.Sock,
		grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		return grpcErrorParse(err)
	}
	defer con.Close()

	cc := cligrpc.NewCliClient(con)
	resp, err := cc.CaseList(context.Background(), req)
	if err != nil {
		return grpcErrorParse(err)
	}

	// t.b.d: install print config

	for _, header := range caseListFmt {
		if header.show {
			fmt.Printf(header.fmt, header.discription)
		}
	}
	fmt.Println()

	for _, testcase := range resp.InfoList {
		item := []interface{}{
			testcase.Name,                         // 0
			testcase.Tag,                          // 1
			testcase.ID[:12],                      // 2
			testcase.CTime,                        // 3
			testcase.ATime,                        // 4
			testcase.MTime,                        // 5
			strconv.FormatUint(testcase.Size, 10), // 6
		}
		for i := range item {
			if caseListFmt[i].show {
				fmt.Printf(caseListFmt[i].fmt, item[i])
			}
		}
		fmt.Println()
	}

	return nil
}
