package cmd // import "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"

import (
	"fmt"
	"os"

	cli "github.com/urfave/cli/v3"
)

var JobCmd = &cli.Command{
	Name:  "job",
	Usage: "Manage jobs",

	Commands: []*cli.Command{
		JobLsCmd,
		JobInspectCmd,
		JobRunCmd,
	},

	ErrWriter: os.Stderr,

	CommandNotFound: func(ctx *cli.Context, command string) {
		fmt.Fprintf(ctx.Command.ErrWriter, "%s: %q is not a %s command.\n", ctx.Command.FullName(), command, ctx.Command.Name)
		cli.ShowSubcommandHelp(ctx)
	},
}
