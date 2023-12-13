package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"context"
	"fmt"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	cli "github.com/urfave/cli/v3"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func TaskList(ctx *cli.Context) error {
	req := &cligrpc.TaskListRequest{}

	if ctx.Args().Len() < 1 {
		return ErrAtLeastOneArgument
	}

	req.JobID = ctx.Args().First()

	con, err := grpc.Dial("unix://"+cfg.Service.CliDaemon.Sock,
		grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		return grpcErrorParse(err)
	}
	defer con.Close()

	cc := cligrpc.NewCliClient(con)
	resp, err := cc.TaskList(context.Background(), req)
	if err != nil {
		return grpcErrorParse(err)
	}

	// t.b.d: install print config

	for _, header := range taskListFmt {
		if header.show {
			fmt.Printf(header.fmt, header.discription)
		}
	}
	fmt.Println()

	statMap := []string{"STANDBY", "RUNNING", "FINISHED", "ERROR"}
	roleMap := []string{"", "HOST", "CONTAINER"}

	for _, task := range resp.InfoList {
		item := []interface{}{
			task.ID[:12],          // 0
			task.CaseInfo.ID[:12], // 1
			task.NodeInfo.ID[:12], // 2
			task.NodeInfo.IP,      // 3
			roleMap[task.Role],    // 4
			task.CTime,            // 5
			task.FTime,            // 6
			statMap[task.Stat],    // 7
		}
		for i := range item {
			if taskListFmt[i].show {
				fmt.Printf(taskListFmt[i].fmt, item[i])
			}
		}
		fmt.Println()
	}

	return nil
}
