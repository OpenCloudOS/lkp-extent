#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "lkpServer.h"
#include "lkpClient.h"
#include "lkpHelper.h"


string ROOT_DIR;

int main(int argc, char *argv[])
{
    
    if(argc!=2){
        std::cout << "lkp-ctl init failed: Wrong argv" << std::endl;
        return 1;
    }

    //需要获得lkp-extent的root direction
    ROOT_DIR = string(argv[1]);

    std::map<string,string> configMap;
    lkpConfig MyConfig;

    //配置初始化
    lkpConfigInit(configMap, MyConfig, ROOT_DIR);
    
    if(!configMap.count("ServerNClient")){
        std::cout << "lkp-extent int failed: Please set whether Server or Client!" << std::endl;
        return 1;
    }

    const bool isServer = ( configMap["ServerNClient"].compare("1")==0 ? true : false);

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

        std::cout << "Continue as a daemon process, pid is " << getpid() << std::endl;
        loop.loop();
    }
    return 0;
}