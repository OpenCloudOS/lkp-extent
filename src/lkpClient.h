#ifndef LKP_CLIENT
#define LKP_CLIENT

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <thread>
#include <set>
#include <atomic>
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

#include "muduo/base/Thread.h"
#include "muduo/base/ThreadPool.h"
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
#include "muduo/base/LogStream.h"
#include "muduo/base/LogFile.h"
#include <sys/stat.h>
#include <boost/shared_ptr.hpp>
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/ThreadLocalSingleton.h"

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
typedef boost::shared_ptr<FILE> FilePtr;

//前端写日志时调用
void asyncOutput(const char *msg, int len);

class lkpClient : boost::noncopyable
{
public:
    lkpClient(EventLoop *loop, const InetAddress &serverAddr, int seconds);

    ~lkpClient();

    //连接服务器
    void connect();
    //断开与服务器的连接
    void disconnect();

private:
    // 该函数在IO线程中执行，IO线程与主线程不在同一个线程
    void onConnection(const TcpConnectionPtr &conn);

    //收到命令的回调函数，server转发给client， client执行
    void onCommandMsg(const TcpConnectionPtr &conn, const RecvCommandPtr &message, Timestamp time);

    //收到result
    void onResult(const TcpConnectionPtr &conn, const RecvCommandPtr &message);

    //每次发送64kb
    void onWriteComplete(const TcpConnectionPtr &conn);
    //收到file message的回调函数，server收到的应该是result， client收到的应该是testcase
    void onFileMsg(const TcpConnectionPtr &conn, const RecvFilePtr &message, Timestamp time);

    //收到未知数据包的回调函数
    void onUnknownMsg(const TcpConnectionPtr &conn, const MessagePtr &message, Timestamp time);

    //定期心跳回调函数
    void onTimer();

    //向服务器发送数据
    void SendToServer(const google::protobuf::Message &messageToSend);

    EventLoop *loop_;
    TcpClient client_;

    lkpDispatcher dispatcher_;
    lkpCodec codec_;

    // MutexLock mutex_;
    TcpConnectionPtr connection_;
    int seconds_;

    int fileSize_; //文件大小
    string fileName_;
    int nodeID_;
    FILE *fp_;
    int kBufSize_;
    pid_t lastPid_;
    string lastCmdString_;
    bool lastStatus_;

    //高速缓冲区使用
    // void threadFunc(); //后端线程，把二级缓冲区写入日志文件
    typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer_log; //缓冲区
    typedef std::vector<std::unique_ptr<Buffer_log>> BufferVector;              //缓冲列表
    typedef BufferVector::value_type BufferPtr;

    const int flushInterval_;     //超时时间，到达时间后没满的缓冲区也必须把数据写入二级缓冲区
    std::atomic<bool> running_;   //保证服务器停止时，后端线程一定停止
    const string basename_;       //输出文件的名称
    const off_t rollSize_;        //日志长度，过长需要roll
    muduo::Thread thread_;        //后端线程
    muduo::CountDownLatch latch_; //确保后端线程启动之后，服务器才开始运行前端线程
    muduo::MutexLock mutex_;
    muduo::Condition cond_ GUARDED_BY(mutex_);   //条件变量，触发后端线程开始写日志
    BufferPtr currentBuffer_ GUARDED_BY(mutex_); //一级当前缓冲区
    BufferPtr nextBuffer_ GUARDED_BY(mutex_);    //一级预备缓冲区
    BufferVector buffers_ GUARDED_BY(mutex_);    //二级缓冲区列表，后端和buffersToWrite交换后，操作buffersToWrite写日志，避免长时间占用buffers_阻塞前端

public:
    //onMessage调用，负责向一级缓冲区写
    void append(const char *logline, int len);

    //后端调用，向日志文件写 muduo::Thread thread_ 绑定该函数，实现后端线程负责写
    void threadFunc();
};

//前端写日志时调用
extern lkpClient *g_asyncLog_client;

void asyncOutput_client(const char *msg, int filelen);

#endif

