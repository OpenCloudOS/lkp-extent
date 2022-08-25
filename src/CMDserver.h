#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <set>
#include <stdio.h>
#include <unistd.h>
#include <atomic>
#include <functional>
#include <vector>
#include <boost/any.hpp>
#include <unordered_map>

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/ThreadLocalSingleton.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/Channel.h"
#include "muduo/net/TimerId.h"


using namespace muduo;
using namespace muduo::net;

#define TCPSERVER "CMD_SUN"
#define CMDIPC "CMDIPC"
#define LEN 4096

//服务器函数
class CMDserver : noncopyable
{
public:
    CMDserver(EventLoop *loop, const InetAddress &listenAddr, int numThreads,int CMDserverSfd);

    void start();

    //向客户端发送数据
    void sendToTcpClient(const StringPiece &message, int nodeID);

    //建立进程间的连接
    void on_accept_CMD_IPC();

    //接收CMDclient的数据
    void recv_CMDclient(int CMDcfd);

private:
    //客户端请求建立新的连接
    void onConnection(const TcpConnectionPtr &conn);

    //接收客户端的数据，发送给CMDclient
    void onMessage(const TcpConnectionPtr &conn,
                              Buffer *buf,
                              Timestamp time);

    TcpServer server_;

    //client pool nodeID -- cfd
    std::unordered_map<int, TcpConnectionPtr> connections_;

    void setThreadNum(int num);

    int numThreads_;
    int nodeCount;

    int CMDserverSfd;

    EventLoop *loop_;
};

//和命令行通信的套接字 server
extern int CMDsfd;

//代表命令行的套接字
extern int CMDcfd;

int buildIPC();

