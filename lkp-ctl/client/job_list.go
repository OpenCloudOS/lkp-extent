package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"context"
	"fmt"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	cli "github.com/urfave/cli/v3"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func JobList(ctx *cli.Context) error {
	req := &cligrpc.JobListRequest{}

	con, err := grpc.Dial("unix://"+cfg.Service.CliDaemon.Sock,
		grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		return grpcErrorParse(err)
	}
	defer con.Close()

	cc := cligrpc.NewCliClient(con)
	resp, err := cc.JobList(context.Background(), req)
	if err != nil {
		return grpcErrorParse(err)
	}

	// t.b.d: install print config

	for _, header := range jobListFmt {
		if header.show {
			fmt.Printf(header.fmt, header.discription)
		}
	}
	fmt.Println()

	statMap := []string{"STANDBY", "RUNNING", "FINISHED", "ERROR"}

	for _, job := range resp.InfoList {
		item := []interface{}{
			job.ID[:12],          // 0
			job.Name,             // 1
			job.CaseInfo.ID[:12], // 2
			job.CTime,            // 3
			job.FTime,            // 4
			statMap[job.Stat],    // 5
			fmt.Sprintf("%d/%d", job.FinishCnt, len(job.Tasks)), // 6
		}
		for i := range item {
			if jobListFmt[i].show {
				fmt.Printf(jobListFmt[i].fmt, item[i])
			}
		}
		fmt.Println()
	}

	return nil
}
