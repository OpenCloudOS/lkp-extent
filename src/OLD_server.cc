#include "OLD_CMDserver.h"

//和命令行通信的套接字 server
int CMDsfd;

//代表命令行的套接字
int CMDcfd;



int main(int argc, char *argv[])
{
    //建立本地进程通信的套接字
    buildIPC();

    if (argc == 1){
        printf("主线程启动，请输入：1.端口号 2.IO线程的数量 3.客户端闲置的最长时间 4.向CMDclient写入的超时时间\n");
        return 0;
    }

    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    int numThreads = 1;
    if (argc > 2)
    {
        numThreads = atoi(argv[2]);
    }
    //客户端闲置的最长时间
    int idleSeconds = 60;
    if(argc > 3){
        idleSeconds = atoi(argv[3]);
    }
    //向CMDclient写入的超时时间
    int flushInterval = 3;
    if(argc > 4){
        flushInterval = atoi(argv[4]);
    }

    InetAddress serverAddr(port);

    off_t kRollSize = 500 * 1000 * 1000;
    CMDserver server(&loop, serverAddr, numThreads,idleSeconds,CMDsfd,kRollSize,flushInterval);


    server.start(); //server_.start() 绝对不能在构造函数里调用，这么做将来会有线程安全的问题
    loop.loop();

    return 0;
}