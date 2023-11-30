package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"errors"

	"google.golang.org/grpc/status"
)

var (
	ErrAtLeastOneArgument  = errors.New("requires at least one argument")
	ErrAtLeastTwoArguments = errors.New("requires at least two arguments")
	ErrDeserialization     = errors.New("deserialization failed")
	ErrCaseFormat          = errors.New("invalid case format")
	ErrRevokeRPC           = errors.New("revoke RPC")
)

func grpcErrorParse(err error) error {
	statusErr, ok := status.FromError(err)

	if ok {
		/*
			if statusErr.Code() == codes.InvalidArgument {
				return fmt.Errorf("invalid argument : %s", statusErr.Message())
			} else {
				return statusErr.Err()
			}
		*/

		return errors.New(statusErr.Message())
	}

	return ErrRevokeRPC
}
