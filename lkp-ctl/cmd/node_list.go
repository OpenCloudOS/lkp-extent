package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var NodeLsCmd = &cli.Command{
	Name:      "ls",
	Usage:     "List lkp-nodes",
	UsageText: "lkp-ctl node ls [OPTIONS]",

	Aliases: []string{"list"},

	Flags: []cli.Flag{
		&cli.BoolFlag{
			Name: "q",
			Aliases: []string{
				"quiet",
			},
			Usage: "Only display lkp-node IDs",
		},
		&cli.BoolFlag{
			Name: "a",
			Aliases: []string{
				"all",
			},
			Usage: "Show all lkp-nodes (default shows active lkp-nodes)",
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

	Action: client.NodeList,
}
