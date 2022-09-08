#include "lkpServer.h"

int main(int argc, char *argv[])
{
    
    uint16_t port = 7777;
    string ip = "127.0.0.1";
    int numThreads = 1;
    int idleSeconds = 5;
    int flushInterval = 3;

    if (argc != 5){
        printf("主线程启动，请输入：1.端口号 2.IO线程的数量 3.客户端闲置的最长时间 4.向CMDclient写入的超时时间\n");
        //return 0;
    }
    else{
        port = static_cast<uint16_t>(atoi(argv[1]));
        numThreads = atoi(argv[2]);
        idleSeconds = atoi(argv[3]);
        flushInterval = atoi(argv[4]);
    }

    //建立本地进程通信的套接字
    EventLoop loop;

    InetAddress serverAddr(port);

    off_t kRollSize = 500 * 1000 * 1000;
    lkpServer Server(&loop, serverAddr, numThreads, idleSeconds, kRollSize, flushInterval);


    Server.start(); //server_.start() 绝对不能在构造函数里调用，这么做将来会有线程安全的问题
    loop.loop();


    return 0;
}