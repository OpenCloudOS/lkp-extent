#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "lkpServer.h"
#include "lkpClient.h"
#include "lkpHelper.h"


string ROOT_DIR;
bool isServer = false;

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

    //创建Server类或client类，开启事件循环并守护运行
    EventLoop loop;
    if(isServer){

        InetAddress serverAddr(MyConfig.ServerListenPort);

        off_t kRollSize = 500 * 1000 * 1000;
        lkpServer Server(&loop, serverAddr, MyConfig, kRollSize);

        g_asyncLog_server = &Server;

        muduo::Logger::setOutput(asyncOutput_server); //LOG_INFO调用asyncOutput_client

        Server.start(); 

        std::cout << "Continue as a daemon process, pid is " << getpid() << std::endl;
        loop.loop();
    }
    else{

        InetAddress serverAddr(MyConfig.ServerAddress, MyConfig.ServerPort);

        lkpClient client(&loop, serverAddr, MyConfig.HeartBeatTime);

        g_asyncLog_client = &client;

        muduo::Logger::setOutput(asyncOutput_client); //LOG_INFO调用asyncOutput_client

        client.connect();

        loop.loop();
    }
    return 0;
}