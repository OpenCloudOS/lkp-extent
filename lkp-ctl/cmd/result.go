package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var ResultCmd = &cli.Command{
	Name:      "result",
	Usage:     "Show the path of test result",
	UsageText: "lkp-ctl result TASK",

	ErrWriter: os.Stderr,

	Action: client.TaskResult,
}
