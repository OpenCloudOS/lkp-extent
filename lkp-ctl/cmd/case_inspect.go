package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var CaseInspectCmd = &cli.Command{
	Name:      "inspect",
	Usage:     "Display detailed information on one or more testcases (default by name:tag)",
	UsageText: "lkp-ctl case inspcet TESTCASE [OPTIONS] [TESTCASE...]",

	UseShortOptionHandling: true,

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

	Action: client.CaseInspect,
}
