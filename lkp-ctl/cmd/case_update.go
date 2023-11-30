package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"os"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cli "github.com/urfave/cli/v3"
)

var CaseUpdateCmd = &cli.Command{
	Name:      "update",
	Usage:     "Update one or more testcases (by name:tag) to the lkp-master",
	UsageText: "lkp-ctl case update TESTCASE [TESTCASE...]",

	ErrWriter: os.Stderr,

	Action: client.CaseUpdate,
}
