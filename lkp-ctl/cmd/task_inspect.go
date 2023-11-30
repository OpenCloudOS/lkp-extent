package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var TaskInspectCmd = &cli.Command{
	Name:      "inspect",
	Usage:     "Display detailed information on one or more tasks",
	UsageText: "lkp-ctl task inspcet TASK [OPTIONS] [TASK...]",

	Flags: []cli.Flag{
		&cli.BoolFlag{
			Name: "y",
			Aliases: []string{
				"yaml",
			},
			Usage: "Display detailed information in yaml format",
		},
	},

	ErrWriter: os.Stderr,

	Action: client.TaskInspect,
}
