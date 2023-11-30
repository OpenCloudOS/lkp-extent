package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

import (
	"fmt"
	"os"

	yaml "gopkg.in/yaml.v2"
)

type Config struct {
	Service struct {
		CliDaemon struct {
			Sock string `yaml:"sock" json:"sock"`
		} `yaml:"cli-daemon" json:"cli-daemon"`
	} `yaml:"service" json:"service"`
}

const (
	cfgPath = "/etc/lkp/cfg.yaml"
)

var cfg Config

func LoadClientConfig() error {
	data, err := os.ReadFile(cfgPath)
	if err != nil {
		return fmt.Errorf("lkp-ctl: init error: %v", err)
	}

	err = yaml.Unmarshal(data, &cfg)
	if err != nil {
		return fmt.Errorf("lkp-ctl: config %q invalid", data)
	}

	if cfg.Service.CliDaemon.Sock == "" {
		cfg.Service.CliDaemon.Sock = "/var/run/lkp.sock"
	}

	return nil
}
