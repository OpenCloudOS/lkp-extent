package main

import (
	"context"
	"fmt"
	"log"
	"net"
	"os"
	"os/exec"
	"os/signal"
	"sync"
	"syscall"

	"lkp-node/proxy"

	"lkp-node/worker"

	"go.uber.org/zap"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/reflection"

	"github.com/AntiBargu/lkp-extent/lkp-node/pkg/taskqueue"
	"github.com/AntiBargu/lkp-extent/lkp-node/task"

	mgrpc "github.com/AntiBargu/lkp-extent/api/monitor"
	schedgrpc "github.com/AntiBargu/lkp-extent/api/sched"
)

func main() {
	logger, err := createLogger()
	if err != nil {
		log.Fatal(err)
	}

	logger.Info("LKP-Node installation completed")

	data, err := os.ReadFile("/etc/lkp/node.yaml")
	if err != nil {
		logger.Fatal("Read cfg failed", zap.Error(err))
	}

	wrk := worker.NewWorker(data)
	wrk.Logger = logger
	wrk.CreateWorkingDirs()
	wrk.TaskQueue = taskqueue.NewTaskQueue()

	wrk.Login()
	defer wrk.Logout()

	p := proxy.NewProxy(data)
	p.Logger = logger
	p.CreateWorkingDirs()
	p.TaskQueue = wrk.TaskQueue

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	wg := sync.WaitGroup{}
	wg.Add(2)

	go startWorker(ctx, &wg, wrk)
	go startProxy(ctx, &wg, p)

	go func() {
		ch := make(chan os.Signal, 1)
		signal.Notify(ch, syscall.SIGINT, syscall.SIGTERM)

		<-ch
		cancel()
	}()

	wg.Wait()
}

func startProxy(ctx context.Context, wg *sync.WaitGroup, p *proxy.Proxy) {
	defer wg.Done()

	listener, err := net.Listen("tcp", ":"+p.Cfg.LKPNode.Proxy.Port)
	if err != nil {
		p.Logger.Fatal("Proxy listen failed", zap.Error(err))
	}
	defer listener.Close()

	s := grpc.NewServer()
	schedgrpc.RegisterSchedServer(s, p)
	reflection.Register(s)

	go func() {
		if err := s.Serve(listener); err != nil {
			p.Logger.Fatal("Proxy start failed", zap.Error(err))
		}
	}()

	p.Logger.Info("Proxy Started", zap.String("listening on", p.Cfg.LKPNode.Proxy.Port))

	<-ctx.Done()

	s.GracefulStop()
}

func startWorker(ctx context.Context, wg *sync.WaitGroup, wrk *worker.Worker) {
	defer wg.Done()

	go func() {
		for {
			item := wrk.TaskQueue.DeQueue()
			t := item.(*task.Task)

			casePath := fmt.Sprintf(wrk.Cfg.Testcase.Path+"%s/%s.yaml", t.CaseID, t.CaseName)
			workPath := fmt.Sprintf("/tmp/lkp/%s/", t.ID)
			rsltPath := fmt.Sprintf(wrk.Cfg.Result.Path+"%s/", t.ID)
			wrk.Logger.Debug("run.sh", zap.String("Case Path", casePath), zap.String("Work Path", workPath),
				zap.String("result Path", rsltPath))

			if t.Role == 1 {
				wrk.Logger.Debug("HOST Testing...", zap.String("Task ID", t.ID))
				exec.Command("/lkp-extent/run.sh", casePath, workPath, rsltPath).Run()
			} else if t.Role == 2 {
				wrk.Logger.Debug("CONTAINER Testing...", zap.String("Task ID", t.ID))
				exec.Command("podman", "run", "-v", "/lkp-tests/:/lkp-tests/", "-v", "/var/run/:/var/run/",
					"antibargu/lkp-extent:opencloudos8.6", "/lkp-extent/run.sh", casePath, workPath, rsltPath).Run()
			} else {
				wrk.Logger.Error("Task role invaild")
			}

			data, err := os.ReadFile(rsltPath + "rslt.tar.gz")
			if err != nil {
				wrk.Logger.Info("Result read failed", zap.String("Result Path", rsltPath+"rslt.tar.gz"), zap.Error(err))
			}

			con, err := grpc.Dial(wrk.Cfg.LKPMaster.Monitor.IP+":"+wrk.Cfg.LKPMaster.Monitor.Port, grpc.WithTransportCredentials(insecure.NewCredentials()))
			if err != nil {
				wrk.Logger.Error("Unable to connect to LKP-Master", zap.Error(err))
			}

			mc := mgrpc.NewMonitorClient(con)

			req := &mgrpc.TaskFinishRequest{
				ID:   t.ID,
				File: data,
			}

			if _, err := mc.TaskFinish(context.Background(), req); err != nil {
				wrk.Logger.Error("Task Finish failed", zap.String("Task ID", t.ID), zap.Error(err))
			}

			con.Close()
		}
	}()

	<-ctx.Done()
}
