#include <iostream>
#include <stdio.h>
#include <string>
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

#include "lkpProto.pb.h"
#include "lkpCodec.h"
#include "lkpDispatcher.h"

using namespace muduo;
using namespace muduo::net;

typedef std::shared_ptr<lkpMessage::PushACK> PushACKPtr;
typedef std::shared_ptr<lkpMessage::CommandACK> CommandACKPtr;
typedef std::shared_ptr<lkpMessage::File> RecvFilePtr;
typedef std::shared_ptr<lkpMessage::Command> RecvCommandPtr;
typedef std::shared_ptr<lkpMessage::HeartBeat> HeartBeatPtr;

google::protobuf::Message* messageToSend;

class lkpClient : boost::noncopyable
{
public:
    lkpClient(EventLoop *loop, const InetAddress &serverAddr,int seconds)
        : loop_(loop),
          client_(loop, serverAddr, "lkpClient"),
          seconds_(seconds),
          dispatcher_(bind(&lkpClient::onUnknownMsg,this,
                    boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)),
          codec_(bind(&lkpDispatcher::onProtobufMessage ,this,
                    boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3))
    {
        //绑定业务回调函数
        dispatcher_.registerMessageCallback<lkpMessage::Command>(bind(&lkpClient::onCommandMsg, 
                            this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
        dispatcher_.registerMessageCallback<lkpMessage::CommandACK>(bind(&lkpClient::onCommandACK, 
                            this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
        dispatcher_.registerMessageCallback<lkpMessage::PushACK>(bind(&lkpClient::onPushACK, 
                            this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
        dispatcher_.registerMessageCallback<lkpMessage::File>(bind(&lkpClient::onFileMsg, 
                            this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
        dispatcher_.registerMessageCallback<lkpMessage::HeartBeat>(bind(&lkpClient::onHeartBeat, 
                            this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));  
                        
        //绑定新连接请求回调函数
        client_.setConnectionCallback(
            bind(&lkpClient::onConnection, this, boost::placeholders::_1));
        //绑定client的信息接收回调函数到lkpCodec
        client_.setMessageCallback(
            bind(&lkpCodec::onMessage, this, 
                boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
       
        //绑定定时器产生心跳包
        loop_->runEvery(seconds_, std::bind(&lkpClient::onTimer, this));

        client_.enableRetry();
    }

    //连接服务器
    void connect()
    {
        client_.connect();
    }

    //断开与服务器的连接
    void disconnect()
    {
        client_.disconnect();
    }

private:
    // 该函数在IO线程中执行，IO线程与主线程不在同一个线程
    void onConnection(const TcpConnectionPtr &conn)
    {
        // mutex用来保护connection_这个shared_ptr
        MutexLockGuard lock(mutex_);
        if (conn->connected())
        {
            connection_ = conn;
        }
        else
        {
            connection_.reset();
        }
    }

    //（弃用）
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                   Timestamp time)
    {
        muduo::StringPiece msg(buf->retrieveAllAsString());
        printf("收到服务器的数据：%s\n", msg.data());

        //TO DO：根据命令内容执行动作
        const muduo::StringPiece str("abcdedf");
        Tcpsend(str);
    }

    //收到命令的回调函数，server转发给client， client执行
    void onCommandMsg(const TcpConnectionPtr &conn, const RecvCommandPtr& message, Timestamp time){
        printf("i have recved a command!\n");
    }

    //收到pushack的回调函数，应该开始发testecase的文件内容
    void onPushACK(const TcpConnectionPtr &conn, const PushACKPtr& message, Timestamp time){

    }

    //收到command ACK的回调函数，应该使统计数量++
    void onCommandACK(const TcpConnectionPtr &conn, const RecvCommandPtr& message, Timestamp time){

    }

    //收到file message的回调函数，server收到的应该是result， client收到的应该是testcase
    void onFileMsg(const TcpConnectionPtr &conn, const RecvFilePtr& message, Timestamp time){
        printf("recv a file msg, file name is %s\n", message->file_name());
    }

    //收到心跳包的回调函数
    void onHeartBeat(const TcpConnectionPtr &conn, const HeartBeatPtr& message, Timestamp time){
        printf("Error!\n");
    }

    //收到未知数据包的回调函数
    void onUnknownMsg(const TcpConnectionPtr &conn, const MessagePtr& message, Timestamp time){

}


    //定期心跳回调函数
    void onTimer()
    {
        MutexLockGuard lock(mutex_);
        if (connection_)
        {
            //TO DO：调用编码函数发送
            connection_->send("OK");
            // codec_.send(get_pointer(connection_), message);
        }
    }

    //向服务器发送数据
    void Tcpsend(const StringPiece &message)
    {
        // mutex用来保护connection_这个shared_ptr
        MutexLockGuard lock(mutex_);
        if (connection_)
        {
            //TO DO：调用编码函数发送
            printf("客户端发送的数据：%s\n", message.data());
            connection_->send(message);
            // codec_.send(get_pointer(connection_), message);
        }
    }

    EventLoop *loop_;
    TcpClient client_;

    lkpDispatcher dispatcher_;
    lkpCodec codec_;

    MutexLock mutex_;
    TcpConnectionPtr connection_;
    int seconds_;
};

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("请输入：1.端口号 2.发送OK的间隔\n");
        return 0;
    }

    int port = 7777;
    if (argc >= 2)
    {
        port = std::stoi(argv[1]);
    }

    int seconds = 1;
    if (argc >= 3)
    {
        seconds = std::stoi(argv[2]);
    }

    EventLoop loop;
    InetAddress serverAddr(argv[1], port);

    lkpClient client(&loop, serverAddr,seconds);
    client.connect();

    loop.loop();
}