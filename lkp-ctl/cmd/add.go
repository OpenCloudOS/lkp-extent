package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var AddCmd = &cli.Command{
	Name:      "add",
	Usage:     "Add one or more testcases (by name:tag) to the lkp-master",
	UsageText: "lkp-ctl add TESTCASE [TESTCASE...]",

	ErrWriter: os.Stderr,

	Action: client.CaseAdd,
}
