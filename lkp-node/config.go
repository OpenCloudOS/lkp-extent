package main

import (
	"fmt"
	"log"
	"os"
	"time"

	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
	"gopkg.in/yaml.v2"
)

const (
	logDir = "/var/log/lkp/"
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
}

func createLogger() (*zap.Logger, error) {
	err := os.MkdirAll(logDir, 0755)
	if err != nil {
		log.Fatal(err)
	}

	logPath := fmt.Sprintf(logDir+"%d.log", time.Now().Unix())

	return zap.Config{
		Level:             zap.NewAtomicLevelAt(zap.DebugLevel),
		OutputPaths:       []string{logPath},
		ErrorOutputPaths:  []string{"stderr"},
		DisableCaller:     false,
		DisableStacktrace: false,

		Encoding: "console",
		EncoderConfig: zapcore.EncoderConfig{
			MessageKey:    "msg",
			LevelKey:      "level",
			TimeKey:       "ts",
			CallerKey:     "caller",
			FunctionKey:   "func",
			StacktraceKey: "stacktrace",
			EncodeLevel:   zapcore.CapitalColorLevelEncoder,
			EncodeTime:    zapcore.ISO8601TimeEncoder,
			EncodeCaller:  zapcore.ShortCallerEncoder,
		},
	}.Build()
}

func loadConfig(data []byte) (*Config, error) {
	cfg := &Config{}

	yaml.Unmarshal(data, cfg)

	if cfg.LKPMaster.Monitor.IP == "" {
		return nil, fmt.Errorf("LKP-Master IP is not configured")
	}

	if cfg.LKPMaster.Monitor.Port == "" {
		return nil, fmt.Errorf("LKP-Master Port is not configured")
	}

	return cfg, nil
}
