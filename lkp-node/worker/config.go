package worker // import "github.com/AntiBargu/lkp-extent/lkp-node/worker"

import (
	yaml "gopkg.in/yaml.v2"
)

type Config struct {
	LKPMaster struct {
		Monitor struct {
			IP   string `json:"ip" yaml:"ip"`
			Port string `json:"port" yaml:"port"`
		} `json:"monitor" yaml:"monitor"`
	} `json:"lkp-master" yaml:"lkp-master"`
	LKPNode struct {
		Proxy struct {
			Port string `json:"port" yaml:"port"`
		} `json:"proxy" yaml:"proxy"`
	} `json:"lkp-node" yaml:"lkp-node"`
	Testcase struct {
		Path string `json:"path" yaml:"path"`
	} `json:"testcase" yaml:"testcase"`
	Result struct {
		Path string `json:"path" yaml:"path"`
	} `json:"result" yaml:"result"`
}

const (
	port       = "28928"
	casePath   = "/var/run/lkp/cases/"
	resultPath = "/var/run/lkp/rslts/"
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

	if cfg.Result.Path == "" {
		cfg.Result.Path = defaultResultPath()
	}

	return cfg
}

func defaultListenPort() string {
	return port
}

func defaultCasePath() string {
	return casePath
}

func defaultResultPath() string {
	return resultPath
}
