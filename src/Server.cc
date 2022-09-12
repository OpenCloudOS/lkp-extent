#include "lkpServer.h"
#include <map>
#include <string>
#include <iostream>

lkpServer *g_asyncLog = NULL;

int main(int argc, char *argv[])
{
    
    uint16_t port = 7777;
    string ip = "127.0.0.1";
    int numThreads = 1;
    int idleSeconds = 5;
    int flushInterval = 3;

    if (argc != 5){
        printf("主线程启动，请输入：1.端口号 2.IO线程的数量 3.客户端闲置的最长时间 4.向CMDclient写入的超时时间\n");
    }
    else{
        port = static_cast<uint16_t>(atoi(argv[1]));
        numThreads = atoi(argv[2]);
        idleSeconds = atoi(argv[3]);
        flushInterval = atoi(argv[4]);
    }

    const string CONFIG_PATH = "lkp-extent.config";
    std::map<string,string> configMap;
    if(!ReadConfig(CONFIG_PATH, configMap)){
        std::cout << "lkp-ctl init failed: Cannot read config file!" << std::endl;
        exit(EXIT_FAILURE);
    }
    else{
        PrintConfig(configMap);
    }
    if(configMap.count("ServerNClient"))
        std::cout << configMap["ServerNClient"] << std::endl;


    //建立本地进程通信的套接字
    EventLoop loop;

    InetAddress serverAddr(port);

    off_t kRollSize = 500 * 1000 * 1000;
    lkpServer Server(&loop, serverAddr, numThreads, idleSeconds, kRollSize, flushInterval);
    g_asyncLog = &Server;

    Server.start(); 
    loop.loop();


    return 0;
}