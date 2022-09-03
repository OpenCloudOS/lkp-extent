#include <muduo/base/Thread.h>
#include <muduo/base/ThreadPool.h>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <iostream>
#include <stdio.h>
#include<string>
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
#include "muduo/base/BlockingQueue.h"
#include "muduo/base/BoundedBlockingQueue.h"
#include "muduo/base/CountDownLatch.h"
#include <boost/circular_buffer.hpp>
#include "muduo/base/LogStream.h"
#include "muduo/base/LogFile.h"


#define LEN 4096

using namespace muduo;
using namespace muduo::net;

class TCPclient : boost::noncopyable
{
public:
    TCPclient(EventLoop *loop, const InetAddress &serverAddr,int seconds)
        : loop_(loop),
          client_(loop, serverAddr, "TCPclient"),
          seconds_(seconds)
        //   codec_(boost::bind(&TCPclient::onStringMessage, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3))
    {
        client_.setConnectionCallback(
            boost::bind(&TCPclient::onConnection, this, boost::placeholders::_1));

        //TO DO：编码发送，解码接收！！！！！！！
        client_.setMessageCallback(
            bind(&TCPclient::onMessage, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));


        //心跳
        loop_->runEvery(seconds_, std::bind(&TCPclient::onTimer, this));

        client_.enableRetry();
    }

    void connect()
    {
        client_.connect();
    }

    void disconnect()
    {
        client_.disconnect();
    }

    //定期心跳
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

    EventLoop *loop_;
    TcpClient client_;

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

    TCPclient client(&loop, serverAddr,seconds);
    client.connect();

    loop.loop();
}