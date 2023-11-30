package main

import (
	"context"
	"fmt"
	"net/mail"
	"os"

	cli "github.com/urfave/cli/v3"

	client "github.com/AntiBargu/lkp-extent/lkp-ctl/client"
	cmd "github.com/AntiBargu/lkp-extent/lkp-ctl/cmd"
)

func main() {
	cmd := &cli.Command{
		// set lkp-ctl CLI features
		Name:        "lkp-ctl",
		Usage:       "A enhance lkp for containers",
		UsageText:   "lkp-ctl COMMAND [OPTIONS]",
		Description: "lkp-ctl is a CLI-tool in LKP-EXTENT project",

		Version: version,
		Authors: []any{
			&mail.Address{
				Name:    author,
				Address: email,
			},
		},

		Flags: []cli.Flag{
			&cli.BoolFlag{
				Name:  "debug, d",
				Usage: "debug mode",
			},
		},

		Commands: []*cli.Command{
			cmd.AddCmd,
			cmd.UpdateCmd,
			cmd.PushCmd,
			cmd.RunCmd,
			cmd.ResultCmd,
			cmd.NodesCmd,
			cmd.CasesCmd,
			cmd.JobsCmd,
			cmd.TasksCmd,
			cmd.NodeCmd,
			cmd.CaseCmd,
			cmd.JobCmd,
			cmd.TaskCmd,
		},

		ErrWriter: os.Stderr,

		Before: func(ctx *cli.Context) error {
			return client.LoadClientConfig()
		},

		CommandNotFound: func(ctx *cli.Context, command string) {
			fmt.Fprintf(ctx.Command.ErrWriter, "%s: %q is not a %s command.\n", ctx.Command.FullName(), command, ctx.Command.Name)
			cli.ShowSubcommandHelp(ctx)
		},

		ExitErrHandler: func(ctx *cli.Context, err error) {
			if err != nil {
				fmt.Fprintf(ctx.Command.ErrWriter, "%q: %s\n", ctx.Command.FullName(), err)
				switch err {
				case client.ErrAtLeastOneArgument:
					cli.ShowSubcommandHelp(ctx)
				case client.ErrAtLeastTwoArguments:
					cli.ShowSubcommandHelp(ctx)
				}
			}
		},
	}

	_ = cmd.Run(context.Background(), os.Args)
}
