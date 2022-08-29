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
#include <unordered_set>

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
#include <boost/circular_buffer.hpp>


using namespace muduo;
using namespace muduo::net;

#define TCPSERVER "CMD_SUN"
#define CMDIPC "CMDIPC"
#define LEN 4096

//服务器函数
class CMDserver : noncopyable
{
public:
    CMDserver(EventLoop *loop, const InetAddress &listenAddr, int numThreads,int idleSeconds,int CMDserverSfd);

    void start();

    //向客户端发送数据
    void sendToTcpClient(const StringPiece &message, int nodeID);

    //建立进程间的连接
    void on_accept_CMD_IPC();

    //接收CMDclient的数据
    void recv_CMDclient(int CMDcfd);

    void onTimer();


    void dumpConnectionBuckets() const;

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


    //time wheeling
    typedef std::weak_ptr<muduo::net::TcpConnection>WeakTcpConnectionPtr;
    //Entry的引用计数变成0时，关闭Entry对应客户端的连接
    struct Entry: public muduo::copyable
    {
        //记录Entry对应的客户端连接，传入的是客户端连接的弱引用
        Entry(const WeakTcpConnectionPtr&weakConn)
        :weakConn_(weakConn){

        }
        
        //计数是0时，关闭客户端的连接
        ~Entry(){
            muduo::net::TcpConnectionPtr conn = weakConn_.lock();//weak_ptr没有*功能，只能这样取，得到的是share_ptr类型的指针
            if(conn){
                conn->shutdown();//引用计数0，关闭连接
            }
        }
        WeakTcpConnectionPtr weakConn_;//弱指针，不会导致计数增加
    };

    typedef std::shared_ptr<Entry> EntryPtr;
    typedef std::weak_ptr<Entry> WeakEntryPtr;
    typedef std::unordered_set<EntryPtr>Bucket;//环形队列的元素
    typedef boost::circular_buffer<Bucket>WeakConnectionList;//环形队列
    WeakConnectionList connectionBuckets_;//环形队列
};

//和命令行通信的套接字 server
extern int CMDsfd;

//代表命令行的套接字
extern int CMDcfd;

int buildIPC();

