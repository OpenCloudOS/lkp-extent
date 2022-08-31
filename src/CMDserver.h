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
#include<set>

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
#include "muduo/base/BlockingQueue.h"
#include "muduo/base/BoundedBlockingQueue.h"
#include "muduo/base/CountDownLatch.h"
#include <boost/circular_buffer.hpp>
#include "muduo/base/Thread.h"
#include "muduo/base/LogStream.h"
#include <atomic>
#include "muduo/base/LogFile.h"


using namespace muduo;
using namespace muduo::net;

#define TCPSERVER "CMD_SUN"
#define CMDIPC "CMDIPC"
#define LEN 4096

//服务器函数
class CMDserver : noncopyable
{
public:
    CMDserver(EventLoop *loop, 
    const InetAddress &listenAddr, int numThreads,int idleSeconds,int sfd,
    off_t rollSize,int flushInterval = 3);

    //logging
    ~CMDserver()
    {
        if (running_)
        {
            running_ = false;//不允许后端继续写日志
            cond_.notify();//避免后端卡在条件变量处
            thread_.join();//回收后端线程
        }
    }
    //logging
    void append(const char *logline, int len); //前端向一级缓冲区添加数据，操作一级缓冲区之前必须加锁
    void threadFunc();//后端操作二级，向CMDcleint发送


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

    std::set<int> idleNodeID;//存放当前闲置的nodeID


    //time wheeling
    typedef std::weak_ptr<muduo::net::TcpConnection>WeakTcpConnectionPtr;

    //Entry的引用计数变成0时，关闭Entry对应客户端的连接
    struct Entry: public muduo::copyable
    {
        //记录Entry对应的客户端连接，传入的是客户端连接的弱引用
        Entry(const WeakTcpConnectionPtr &weakConn, CMDserver*server,int nodeID)
            : weakConn_(weakConn),Entry_server(server),Entry_nodeID(nodeID)
        {
            // printf("Entry，nodeID:%d\n",Entry_nodeID);
        }

        //计数是0时，关闭客户端的连接
        ~Entry(){
            // printf("~Entry，nodeID:%d\n",Entry_nodeID);

            muduo::net::TcpConnectionPtr conn = weakConn_.lock();//weak_ptr没有*功能，只能这样取，得到的是share_ptr类型的指针
            if(conn){
                conn->shutdown();//引用计数0，关闭连接
            }
            else{
                return;
            }

            // printf("~Entry，shutdown\n");
            //关闭客户端连接时，产生空闲nodeID
            Entry_server->idleNodeID.insert(Entry_nodeID);
            // printf("~Entry，insert\n");
        }
        WeakTcpConnectionPtr weakConn_;//弱指针，不会导致计数增加
        CMDserver* Entry_server;//存放Entry对应的CMDserver
        int Entry_nodeID;
    };

    typedef std::shared_ptr<Entry> EntryPtr;
    typedef std::weak_ptr<Entry> WeakEntryPtr;
    typedef std::unordered_set<EntryPtr>Bucket;//环形队列的元素
    typedef boost::circular_buffer<Bucket>WeakConnectionList;//环形队列
    WeakConnectionList connectionBuckets_;//环形队列


    //logging
    // void threadFunc(); //后端线程，把二级缓冲区写入日志文件
    typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer_log;//缓冲区
    typedef std::vector<std::unique_ptr<Buffer_log>> BufferVector;//缓冲列表
    typedef BufferVector::value_type BufferPtr;

    const int flushInterval_; //超时时间，到达时间后没满的缓冲区也必须把数据写入二级缓冲区
    std::atomic<bool> running_;//保证服务器停止时，后端线程一定停止
    const string basename_;//输出文件的名称
    const off_t rollSize_;//日志长度，过长需要roll
    muduo::Thread thread_;//后端线程
    muduo::CountDownLatch latch_; //确保后端线程启动之后，服务器才开始运行前端线程
    muduo::MutexLock mutex_;
    muduo::Condition cond_ GUARDED_BY(mutex_);//条件变量，触发后端线程开始写日志
    BufferPtr currentBuffer_ GUARDED_BY(mutex_); //一级当前缓冲区
    BufferPtr nextBuffer_ GUARDED_BY(mutex_);    //一级预备缓冲区
    BufferVector buffers_ GUARDED_BY(mutex_);    //二级缓冲区列表，后端和buffersToWrite交换后，操作buffersToWrite写日志，避免长时间占用buffers_阻塞前端
};

//和命令行通信的套接字 server
extern int CMDsfd;

//代表命令行的套接字
extern int CMDcfd;

int buildIPC();

