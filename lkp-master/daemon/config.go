package daemon // import "github.com/AntiBargu/lkp-extent/lkp-master/daemon"

import (
	yaml "gopkg.in/yaml.v2"
)

type Config struct {
	Service struct {
		CliDaemon struct {
			Sock string `yaml:"sock" json:"sock"`
			Port string `yaml:"port" json:"port"`
		} `yaml:"cli-daemon" json:"cli-daemon"`
	} `yaml:"service" json:"service"`

	Feature struct {
		Timestamp struct {
			Format string `yaml:"fmt" json:"fmt"`
		} `yaml:"timestamp" json:"timestamp"`
	} `yaml:"feature" json:"feature"`
}

const (
	sockPath = "/var/run/lkp.sock"
	port     = "8999"

	casePath = "/var/run/lkpd/cases/"
	taskPath = "/var/run/lkpd/rslts/"

	timestamp = "2006-01-02 15:04:05"
)

func loadConfig(data []byte) *Config {
	cfg := &Config{}

	yaml.Unmarshal(data, cfg)

	if cfg.Service.CliDaemon.Sock == "" {
		cfg.Service.CliDaemon.Sock = defaultListenSock()
	}

	if cfg.Service.CliDaemon.Port == "" {
		cfg.Service.CliDaemon.Port = defaultListenPort()
	}

	if cfg.Feature.Timestamp.Format == "" {
		cfg.Feature.Timestamp.Format = defaultTimestamp()
	}

	return cfg
}

func defaultListenSock() string {
	return sockPath
}

func defaultListenPort() string {
	return port
}

func defaultTimestamp() string {
	return timestamp
}
