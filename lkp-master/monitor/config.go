package monitor // import "github.com/AntiBargu/lkp-extent/lkp-master/monitor"

import (
	yaml "gopkg.in/yaml.v2"
)

type Config struct {
	Service struct {
		Monitor struct {
			Port             string `yaml:"port" json:"port"`
			Keepalive        int64  `yaml:"keepalive" json:"keepalive"`
			KeepaliveTimeout int64  `yaml:"keepalivetimeout" json:"keepalivetimeout"`
		} `yaml:"monitor" json:"monitor"`
	} `yaml:"service" json:"service"`

	Resource struct {
		Task struct {
			Path string `yaml:"path" json:"path"`
		} `yaml:"task" json:"task"`
	} `yaml:"resource" json:"resource"`
}

const (
	port     = "8883"
	taskPath = "/var/run/lkpd/rslts/"
)

func loadConfig(data []byte) *Config {
	cfg := &Config{}

	yaml.Unmarshal(data, cfg)

	if cfg.Service.Monitor.Port == "" {
		cfg.Service.Monitor.Port = defaultListenPort()
	}

	if cfg.Resource.Task.Path == "" {
		cfg.Resource.Task.Path = defaultTaskPath()
	}

	return cfg
}

func defaultListenPort() string {
	return port
}

func defaultTaskPath() string {
	return taskPath
}
