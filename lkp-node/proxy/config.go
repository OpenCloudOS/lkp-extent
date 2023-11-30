package proxy // import "github.com/AntiBargu/lkp-extent/lkp-node/proxy"

import (
	yaml "gopkg.in/yaml.v2"
)

type Config struct {
	Testcase struct {
		Path string `json:"path" yaml:"path"`
	} `json:"testcase" yaml:"testcase"`
	LKPNode struct {
		Proxy struct {
			Port string `json:"port" yaml:"port"`
		} `json:"proxy" yaml:"proxy"`
	} `json:"lkp-node" yaml:"lkp-node"`
}

const (
	port     = "28928"
	casePath = "/var/run/lkp/cases/"
)

func loadConfig(data []byte) *Config {
	cfg := &Config{}

	yaml.Unmarshal(data, cfg)

	if cfg.LKPNode.Proxy.Port == "" {
		cfg.LKPNode.Proxy.Port = defaultListenPort()
	}

	if cfg.Testcase.Path == "" {
		cfg.Testcase.Path = defaultCasePath()
	}

	return cfg
}

func defaultListenPort() string {
	return port
}

func defaultCasePath() string {
	return casePath
}
