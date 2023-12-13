package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"fmt"
	"os"

	cli "github.com/urfave/cli/v3"
)

var CaseCmd = &cli.Command{
	Name:      "case",
	Usage:     "Manage testcases",
	UsageText: "lkp-ctl case COMMAND",

	Commands: []*cli.Command{
		CaseAddCmd,
		CaseUpdateCmd,
		CasePushCmd,
		CaseLsCmd,
		CaseInspectCmd,
	},

	ErrWriter: os.Stderr,

	CommandNotFound: func(ctx *cli.Context, command string) {
		fmt.Fprintf(ctx.Command.ErrWriter, "%s: %q is not a %s command.\n", ctx.Command.FullName(), command, ctx.Command.Name)
		cli.ShowSubcommandHelp(ctx)
	},
}
