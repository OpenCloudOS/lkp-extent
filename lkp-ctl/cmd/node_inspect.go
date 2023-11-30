package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var NodeInspectCmd = &cli.Command{
	Name:      "inspect",
	Usage:     "Display detailed information on one or more lkp-nodes (default by json format)",
	UsageText: "lkp-ctl node inspcet NODE [OPTIONS] [NODE...]",

	Flags: []cli.Flag{
		&cli.BoolFlag{
			Name: "y",
			Aliases: []string{
				"yaml",
			},
			Usage: "Display detailed information by yaml format",
		},
	},

	ErrWriter: os.Stderr,

	Action: client.NodeInspect,
}
