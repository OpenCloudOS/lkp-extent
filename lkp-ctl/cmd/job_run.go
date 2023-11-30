package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var JobRunCmd = &cli.Command{
	Name:      "run",
	Usage:     "Run the testcase on one or more lkp-nodes",
	UsageText: "lkp-ctl job run TESTCASE [OPTIONS] [NODE...]",

	Flags: []cli.Flag{
		&cli.BoolFlag{
			Name: "a",
			Aliases: []string{
				"all",
			},
			Usage: "Run the testcase on all lkp-nodes",
		},
		&cli.IntFlag{
			Name: "c",
			Aliases: []string{
				"container",
			},
			Usage: "Set the number of containers per node",
			Value: 0,
		},
		&cli.StringFlag{
			Name: "n",
			Aliases: []string{
				"name",
			},
			Usage: "Assign a name to the job",
			Value: "",
		},
	},

	ErrWriter: os.Stderr,

	Action: client.JobRun,
}
