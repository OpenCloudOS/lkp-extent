package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"fmt"
	"os"

	cli "github.com/urfave/cli/v3"
)

var TaskCmd = &cli.Command{
	Name:  "task",
	Usage: "Manage tasks",

	Commands: []*cli.Command{
		TaskLsCmd,
		TaskInspectCmd,
		TaskResultCmd,
	},

	ErrWriter: os.Stderr,

	CommandNotFound: func(ctx *cli.Context, command string) {
		fmt.Fprintf(ctx.Command.ErrWriter, "%s: %q is not a %s command.\n", ctx.Command.FullName(), command, ctx.Command.Name)
		cli.ShowSubcommandHelp(ctx)
	},
}
