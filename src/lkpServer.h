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
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <atomic>
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

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
#include "muduo/base/LogFile.h"

#include "lib/lkpProto.pb.h"
#include "lib/lkpCodec.h"
#include "lib/lkpDispatcher.h"
#include "lkpHelper.h"

using namespace muduo;
using namespace muduo::net;


typedef std::shared_ptr<lkpMessage::PushACK> PushACKPtr;
typedef std::shared_ptr<lkpMessage::CommandACK> CommandACKPtr;
typedef std::shared_ptr<lkpMessage::File> RecvFilePtr;
typedef std::shared_ptr<lkpMessage::Command> RecvCommandPtr;
typedef std::shared_ptr<lkpMessage::HeartBeat> HeartBeatPtr;

class lkpServer : noncopyable
{
public:
    lkpServer(EventLoop *loop,
              const InetAddress &listenAddr, int numThreads, int idleSeconds,
              off_t rollSize, int flushInterval = 3);

    ~lkpServer()
    {
        if (running_)
        {
            running_ = false;//不允许后端继续写日志
            cond_.notify();//避免后端卡在条件变量处
            thread_.join();//回收后端线程
        }
    }

    //启动服务器
    void start();

    //设置线程数量
    void setThreadNum(int num);

private:

    //向客户端发送数据
    void SendToClient(const google::protobuf::Message& message, int nodeID);
    //向命令行客户端发送数据
    void SendToCmdClient(const google::protobuf::Message& message);

    // IPC 相关函数
    // 建立进程间的连接
    void onAcceptIPC();
    //接收CMDclient的数据
    void onCMDmessage(int CMDcfd);


    void onTimer();

    //客户端请求建立新的连接
    void onConnection(const TcpConnectionPtr &conn);

    //取消使用。
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);

    //收到命令的回调函数，server转发给client， client执行
    void onCommandMsg(const TcpConnectionPtr &conn, const RecvCommandPtr& message, Timestamp time);
    //收到pushack的回调函数，应该开始发testecase的文件内容
    void onPushACK(const TcpConnectionPtr &conn, const PushACKPtr& message, Timestamp time);
    //收到command ACK的回调函数，应该使统计数量++
    void onCommandACK(const TcpConnectionPtr &conn, const CommandACKPtr& message, Timestamp time);
    //收到file message的回调函数，server收到的应该是result， client收到的应该是testcase
    void onFileMsg(const TcpConnectionPtr &conn, const RecvFilePtr& message, Timestamp time);
    //收到心跳包的回调函数
    void onHeartBeat(const TcpConnectionPtr &conn, const HeartBeatPtr& message, Timestamp time);
    //收到未知数据包的回调函数
    void onUnknownMsg(const TcpConnectionPtr &conn, const MessagePtr& message, Timestamp time);


    void dumpConnectionBuckets() const;

    void append(const char *logline, int len); //前端向一级缓冲区添加数据，操作一级缓冲区之前必须加锁
    void threadFunc();//后端操作二级，向CMDcleint发送

    TcpServer server_;
    EventLoop *loop_;

    int numThreads_;

    lkpDispatcher dispatcher_;
    lkpCodec codec_;

    TcpConnectionPtr CmdConnection_;
    bool hasCmdConnected_ = false;
    
    //client pool nodeID -- cfd
    std::unordered_map<int, TcpConnectionPtr> connections_;
    int nodeCount_;
    std::set<int> idleNodeID;//存放当前闲置的nodeID

    //time wheeling使用
    typedef std::weak_ptr<muduo::net::TcpConnection>WeakTcpConnectionPtr;
    //Entry的引用计数变成0时，关闭Entry对应客户端的连接
    struct Entry: public muduo::copyable
    {
        //记录Entry对应的客户端连接，传入的是客户端连接的弱引用
        Entry(const WeakTcpConnectionPtr &weakConn, lkpServer*server,int nodeID)
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
        lkpServer* Entry_server;//存放Entry对应的CMDserver
        int Entry_nodeID;
    };

    typedef std::shared_ptr<Entry> EntryPtr;
    typedef std::weak_ptr<Entry> WeakEntryPtr;
    typedef std::unordered_set<EntryPtr> Bucket;//环形队列的元素
    typedef boost::circular_buffer<Bucket> WeakConnectionList;//环形队列
    WeakConnectionList connectionBuckets_;//环形队列


    //高速缓冲区使用
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

