package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var CasePushCmd = &cli.Command{
	Name:      "push",
	Usage:     "Push the testcase to one or more lkp-nodes",
	UsageText: "lkp-ctl case push TESTCASE [OPTIONS] [NODE...]",

	Flags: []cli.Flag{
		&cli.BoolFlag{
			Name: "a",
			Aliases: []string{
				"all",
			},
			Usage: "Push the testcase to all lkp-nodes",
		},
	},

	ErrWriter: os.Stderr,

	Action: client.CasePush,
}
