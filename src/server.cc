#include "CMDserver.h"

//和命令行通信的套接字 server
int CMDsfd;

//代表命令行的套接字
int CMDcfd;



int main(int argc, char *argv[])
{
    //建立进程通信的套接字
    buildIPC();

    if (argc == 1){
        printf("主线程启动，请输入：端口号 IO线程的数量 客户端闲置的最长时间\n");
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

    InetAddress serverAddr(port);
    CMDserver server(&loop, serverAddr, numThreads,idleSeconds,CMDsfd);


    server.start(); //server_.start() 绝对不能在构造函数里调用，这么做将来会有线程安全的问题
    loop.loop();

    return 0;
}