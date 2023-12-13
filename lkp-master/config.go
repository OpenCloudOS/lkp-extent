package main

import (
	"fmt"
	"log"
	"os"
	"time"

	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

const (
	logDir  = "/var/log/lkpd/"
	caseDir = "/var/run/lkpd/cases/"
	rsltDir = "/var/run/lkpd/rslts/"
)

func createRuntimeDirs() {
	dirs := []string{
		logDir,
		caseDir,
		rsltDir,
	}

	for _, dir := range dirs {
		err := os.MkdirAll(dir, 0755)
		if err != nil {
			log.Fatal(err)
		}
	}
}

func createLogger() (*zap.Logger, error) {
	now := time.Now().Unix()
	logFile := fmt.Sprintf(logDir+"%d.log", now)

	return zap.Config{
		Level:             zap.NewAtomicLevelAt(zap.DebugLevel),
		OutputPaths:       []string{logFile},
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
