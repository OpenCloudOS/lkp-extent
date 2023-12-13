package main

import (
	"context"
	"log"
	"net"
	"os"
	"os/signal"
	"sync"
	"syscall"

	"go.uber.org/zap"
	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"

	"github.com/AntiBargu/lkp-extent/lkp-master/daemon"
	"github.com/AntiBargu/lkp-extent/lkp-master/monitor"

	cligrpc "github.com/AntiBargu/lkp-extent/api/cli"
	mgrpc "github.com/AntiBargu/lkp-extent/api/monitor"
)

func init() {
	createRuntimeDirs()
}

func main() {
	logger, err := createLogger()
	if err != nil {
		log.Fatal(err)
	}
	logger.Info("Starting LKP-Master")

	cfg, err := os.ReadFile("/etc/lkp/cfg.yaml")
	if err != nil {
		logger.Fatal("Load cfg failed", zap.Error(err))
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	wg := sync.WaitGroup{}
	wg.Add(2)

	d := daemon.NewDaemon(cfg)
	d.Logger = logger

	go startDaemon(ctx, &wg, d)

	m := monitor.NewMonitor(cfg)
	m.Logger = logger

	go startMonitor(ctx, &wg, m)

	go func() {
		ch := make(chan os.Signal, 1)
		signal.Notify(ch, syscall.SIGINT, syscall.SIGTERM)

		<-ch
		cancel()
	}()

	wg.Wait()
}

func startDaemon(ctx context.Context, wg *sync.WaitGroup, d *daemon.Daemon) {
	defer wg.Done()

	listener, err := net.Listen("unix", d.Cfg.Service.CliDaemon.Sock)
	if err != nil {
		d.Logger.Fatal("Daemon listen failed", zap.Error(err))
	}
	defer listener.Close()

	s := grpc.NewServer()
	cligrpc.RegisterCliServer(s, d)
	reflection.Register(s)

	go func() {
		if err := s.Serve(listener); err != nil {
			d.Logger.Fatal("Daemon start failed", zap.Error(err))
		}
	}()

	d.Logger.Info("Daemon Started", zap.String("listening on", d.Cfg.Service.CliDaemon.Sock))

	<-ctx.Done()

	s.GracefulStop()
}

func startMonitor(ctx context.Context, wg *sync.WaitGroup, m *monitor.Monitor) {
	defer wg.Done()

	listener, err := net.Listen("tcp", ":"+m.Cfg.Service.Monitor.Port)
	if err != nil {
		m.Logger.Fatal("Monitor listen failed", zap.Error(err))
	}
	defer listener.Close()

	s := grpc.NewServer()
	mgrpc.RegisterMonitorServer(s, m)
	reflection.Register(s)

	go func() {
		if err := s.Serve(listener); err != nil {
			m.Logger.Fatal("Monitor start failed", zap.Error(err))
		}
	}()

	m.Logger.Info("Monitor Started", zap.String("listening on", m.Cfg.Service.Monitor.Port))

	<-ctx.Done()

	s.GracefulStop()
}
