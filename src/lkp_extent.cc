#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "lkpServer.h"
#include "lkpClient.h"
#include "lkpHelper.h"


string ROOT_DIR;
bool isServer = false;
int childNum = 0;

//回收子进程
void handleChildExit(int sig){
    int childStatus;
    childNum = std::max(childNum - 1,0);
    //一旦被唤醒，就持续尝试回收子进程
    printf("try to handleChildExit\n");
    while(waitpid(-1,&childStatus,WNOHANG) != -1);
}

int main(int argc, char *argv[])
{
    
    if(argc!=3){
        std::cout << "lkp-ctl service start failed: Wrong argv" << std::endl;
        return 1;
    }

    //需要获得lkp-extent的root direction
    ROOT_DIR = string(argv[1]);
    string ServerNClient = string(argv[2]);

    std::map<string,string> configMap;
    lkpConfig MyConfig;

    //配置初始化
    lkpConfigInit(configMap, MyConfig, ROOT_DIR);

    if(ServerNClient.compare("server")==0)
        isServer = true;
    else if(ServerNClient.compare("client")==0)
        isServer = false;
    else{
        std::cout << "lkp-ctl service start failed: Not a valid input (server or client)" << std::endl;
        return 0;
    }

    if(daemon(1,1) < 0){
        perror("lkp-extent error: cannot run as daemon!" );
        exit(EXIT_FAILURE);
    }
            

    //注册子进程回收函数，由主线程负责
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGCHLD);
    sigprocmask(SIG_BLOCK,&set,NULL);
    struct sigaction act, oldact;
    act.sa_handler = handleChildExit;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, &oldact);
    sigprocmask(SIG_UNBLOCK,&set,NULL);
    

    //创建Server类或client类，开启事件循环并守护运行
    
    if(isServer){
        EventLoop loop;
        InetAddress serverAddr(MyConfig.ServerListenPort);

        off_t kRollSize = 500 * 1000 * 1000;
        lkpServer Server(&loop, serverAddr, MyConfig, kRollSize);

        g_asyncLog_server = &Server;

        muduo::Logger::setOutput(asyncOutput_server); //LOG_INFO调用asyncOutput_client

        Server.start(); 

        std::cout << "lkp-extent server start success! " << std::endl;
        
        loop.loop();
    }
    else{
        EventLoop loop;
        InetAddress serverAddr(MyConfig.ServerAddress, MyConfig.ServerPort);

        lkpClient client(&loop, serverAddr, MyConfig.HeartBeatTime);

        g_asyncLog_client = &client;

        muduo::Logger::setOutput(asyncOutput_client); //LOG_INFO调用asyncOutput_client

        client.connect();

        loop.loop();
    }
    return 0;
}