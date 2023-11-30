package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var TaskLsCmd = &cli.Command{
	Name:      "ls",
	Usage:     "List tasks",
	UsageText: "lkp-ctl task ls JOB [OPTIONS]",

	Aliases: []string{"list"},

	Flags: []cli.Flag{
		&cli.BoolFlag{
			Name: "a",
			Aliases: []string{
				"all",
			},
			Usage: "Show all tasks (default shows last ${LIMIT} tasks)",
		},
		&cli.StringFlag{
			Name: "f",
			Aliases: []string{
				"filter",
			},
			Usage: "Filter output based on conditions provided",
		},
	},

	ErrWriter: os.Stderr,

	Action: client.TaskList,
}
