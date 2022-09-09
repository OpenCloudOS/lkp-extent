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
#include <sys/stat.h>

#include "lib/lkpProto.pb.h"
#include "lib/lkpCodec.h"
#include "lib/lkpDispatcher.h"

using namespace muduo;
using namespace muduo::net;

typedef std::shared_ptr<lkpMessage::PushACK> PushACKPtr;
typedef std::shared_ptr<lkpMessage::CommandACK> CommandACKPtr;
typedef std::shared_ptr<lkpMessage::File> RecvFilePtr;
typedef std::shared_ptr<lkpMessage::Command> RecvCommandPtr;
typedef std::shared_ptr<lkpMessage::HeartBeat> HeartBeatPtr;


class lkpClient : boost::noncopyable
{
public:
    lkpClient(EventLoop *loop, const InetAddress &serverAddr,int seconds)
        : loop_(loop),
          client_(loop, serverAddr, "lkpClient"),
          seconds_(seconds),
          dispatcher_(bind(&lkpClient::onUnknownMsg, this,
                    boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)),
          codec_(bind(&lkpDispatcher::onProtobufMessage, &dispatcher_,
                    boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3))
    {
        //绑定业务回调函数
        dispatcher_.registerMessageCallback<lkpMessage::Command>(bind(&lkpClient::onCommandMsg, 
                            this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
        dispatcher_.registerMessageCallback<lkpMessage::File>(bind(&lkpClient::onFileMsg, 
                            this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
                        
        //绑定新连接请求回调函数
        client_.setConnectionCallback(
            bind(&lkpClient::onConnection, this, boost::placeholders::_1));
        //绑定client的信息接收回调函数到lkpCodec
        client_.setMessageCallback(
            bind(&lkpCodec::onMessage, &codec_, 
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

    //收到命令的回调函数，server转发给client， client执行
    void onCommandMsg(const TcpConnectionPtr &conn, const RecvCommandPtr& message, Timestamp time)
    {   
        //需要回复的ACK
        switch(message->command()){
            case lkpMessage::commandID::UPDATE:{
                //To DO:
                //sh: lkp-ctl update
                lkpMessage::CommandACK ACK;
                ACK.set_command(message->command());
                ACK.set_status(true);
                SendToServer(ACK);
                break;
            }    
            case lkpMessage::commandID::RUN:{
                
                lkpMessage::CommandACK ACK;
                ACK.set_command(message->command());
                string testname = message->testcase();
                if(message->docker_num()){
                    //To DO:
                    // sh: lkp-ctl run $testname -c $dockernum
                }
                else{
                    //To DO:
                    // sh: lkp-ctl run $testname
                }
                ACK.set_status(true);
                SendToServer(ACK);
                break;
            }
            case lkpMessage::commandID::RESULT:{
                lkpMessage::CommandACK ACK;
                ACK.set_command(message->command());
                //To DO:
                //send result to Server
                //sendFile(string filename);
                ACK.set_status(true);
                SendToServer(ACK);
                break;
            }
            case lkpMessage::commandID::PUSH:{
                lkpMessage::PushACK PACK;
                //To DO:
                //bool canRecvFile(uint32 file_len);
                //bool createFile(string file_name);
                PACK.set_status(true);
                SendToServer(PACK);
                break;
            }
        }

    }


    //收到file message的回调函数，server收到的应该是result， client收到的应该是testcase
    void onFileMsg(const TcpConnectionPtr &conn, const RecvFilePtr& message, Timestamp time)
    {
        //文件发送结束
        if (message->file_type() == lkpMessage::File::END)
        {
            ::fclose(fp_);//必须关闭，不然会错误
            
            //获取文件的大小
            struct stat statbuf;
            stat(fileName_.c_str(), &statbuf);
            int recvSize = statbuf.st_size;

            //检查文件是否完整
            if(recvSize != fileSize_){
                printf("recvSize:%d,fileSize_:%d\n",recvSize,fileSize_);
                printf("file is not complete!\n");
                
                //失败回复 
                lkpMessage::PushACK ack;
                ack.set_status(false);
                ack.set_ack_message("push recv fail");
                ack.set_node_id(nodeID_);
                SendToServer(ack);
                return;
            }
            else{
                printf("recv a complete file\n");

                //成功
                lkpMessage::PushACK ack;
                ack.set_status(true);
                ack.set_ack_message("push recv success");
                ack.set_node_id(nodeID_);
                SendToServer(ack);
                return;
            }
        }
        //第一次接收
        else if (message->first_patch())
        {
            nodeID_ = stoi(message->file_name());
            fileName_ = "./testcase/node" + std::to_string(nodeID_) + "/client_testcase";
            printf("fileName_:%s\n",fileName_.c_str());

            fileSize_ = message->file_size();
            fp_ = ::fopen(fileName_.c_str(), "we");
            assert(fp_);
        }

        //每次接收的都输出
        fwrite(message->content().c_str(), 1, message->patch_len(), fp_);
    }

    //收到未知数据包的回调函数
    void onUnknownMsg(const TcpConnectionPtr &conn, const MessagePtr& message, Timestamp time)
    {
        printf("Error!\n");
    }


    //定期心跳回调函数
    void onTimer()
    {
        lkpMessage::HeartBeat heart;
        heart.set_status(true);
        SendToServer(heart);
    }

    //向服务器发送数据
    void SendToServer(const google::protobuf::Message& messageToSend)
    {
        // mutex用来保护connection_这个shared_ptr
        MutexLockGuard lock(mutex_);
        if (connection_->connected())
        {
            codec_.send(connection_, messageToSend);
        }
    }

    EventLoop *loop_;
    TcpClient client_;

    lkpDispatcher dispatcher_;
    lkpCodec codec_;

    MutexLock mutex_;
    TcpConnectionPtr connection_;
    int seconds_;

    int fileSize_;//文件大小
    string fileName_;
    int nodeID_;
    FILE *fp_;
};

int main(int argc, char *argv[])
{
    uint16_t port = 7777;
    int seconds = 3;
    string ip = "192.168.80.128";

    if (argc != 3)
    {
        printf("请输入：1.端口号 2.发送OK的间隔 注意：修改ip为自己的网卡地址！！！！！！！！！！！！\n");
    }
    else {
        ip = argv[1];
        port = static_cast<uint16_t>(atoi(argv[2]));
        seconds = atoi(argv[3]);
    }

    EventLoop loop;
    InetAddress serverAddr(ip, port);

    lkpClient client(&loop, serverAddr,seconds);
    client.connect();
    loop.loop();
}