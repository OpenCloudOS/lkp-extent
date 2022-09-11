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

class lkpClient : boost::noncopyable
{
public:
    lkpClient(EventLoop *loop, const InetAddress &serverAddr,int seconds)
        : loop_(loop),
          client_(loop, serverAddr, "lkpClient"),
          seconds_(seconds),
          kBufSize_(64*1024),
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
        lkpMessage::CommandACK ACK;
        ACK.set_command(message->command());
        ACK.set_node_id(message->node_id());
        int status = 0;

        //如果上一个命令还没有运行完
        if(lastPid_>0 && waitpid(lastPid_, &status, WNOHANG)==0){
            ACK.set_status(false);
            ACK.set_ack_message("ERROR 10: Command " + lastCmdString_ + " is still ruunning!");
            SendToServer(ACK);
            return;
        }

        lkpEnumToCmds(message->command(), lastCmdString_);

        switch(message->command()){

            case lkpMessage::UPDATE:{
                pid_t pid = fork();
                if(pid<0){
                    ACK.set_status(false);
                    ACK.set_ack_message("ERROR 11: Fork Error!");
                    SendToServer(ACK);
                    return;
                }
                //开启新进程执行命令
                if(pid==0){
                    if(execlp("lkp-ctl", "lkp-ctl", "update", NULL)<0){
                        perror("Error on UPDATE exec:");
                        exit(0);
                    }
                }
                else{
                    lastPid_ = pid;
                    if(waitpid(pid, &status, 0)==-1){
                        ACK.set_status(false);
                        ACK.set_ack_message("ERROR 12: Command UPDATE cannot run!");
                        SendToServer(ACK);
                    }
                    else{
                        ACK.set_status(true);
                        SendToServer(ACK);
                    }
                }
                break;
            }    
            case lkpMessage::RUN:{
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

            case lkpMessage::RESULT:{
                onResult(conn,message);
                ACK.set_status(true);
                SendToServer(ACK);
                break;
            }

            case lkpMessage::PUSH:{
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

    //收到result
    void onResult(const TcpConnectionPtr &conn,const RecvCommandPtr& message){
        nodeID_ = message->node_id();
        
        string fileName = "./testcase/node" + std::to_string(nodeID_) + "/" + message->testcase();
        printf("result fileName:%s\n",fileName.c_str());

        //获取文件的大小
        struct stat statbuf;
        stat(fileName.c_str(), &statbuf);
        int fileSize = statbuf.st_size;

        FILE *fp = ::fopen(fileName.c_str(), "rb"); //打开文件
        if (!fp)
        {
            perror("open file fail!!\n");
            return;
        }

        FilePtr ctx(fp, ::fclose); //多了::fclose参数，表示fp对象的引用计数器变成0时，调用::fclose来销毁fp
        conn->setContext(ctx);//把TcpConnectionPtr对象与fp绑定 
        char buf[kBufSize_];
        size_t nread = ::fread(buf, 1, sizeof buf, fp); //读取kBufSize的内容

        lkpMessage::File fileMessage;
        fileMessage.set_file_type(lkpMessage::File::RESULT);
        fileMessage.set_file_name(std::to_string(nodeID_));
        fileMessage.set_file_size(fileSize);
        fileMessage.set_patch_len(nread);
        fileMessage.set_first_patch(true);
        fileMessage.set_content(buf);

        conn->setWriteCompleteCallback(bind(&lkpClient::onWriteComplete, this, boost::placeholders::_1)); //发完一次后继续发
        SendToServer(fileMessage);
    }

    //每次发送64kb
    void onWriteComplete(const TcpConnectionPtr &conn)
    {
        const FilePtr &fp = boost::any_cast<const FilePtr &>(conn->getContext());

        char buf[kBufSize_];
        size_t nread = ::fread(buf, 1, sizeof(buf), get_pointer(fp));

        //续传
        if (nread > 0)
        {
            lkpMessage::File fileMessage;
            fileMessage.set_file_type(lkpMessage::File::RESULT);
            fileMessage.set_first_patch(false);
            fileMessage.set_patch_len(nread);
            fileMessage.set_content(buf);

            SendToServer(fileMessage);
        }
        //结束
        else
        {
            //发送结束信号
            lkpMessage::File fileMessage;
            fileMessage.set_file_type(lkpMessage::File::END);

            conn->setWriteCompleteCallback(NULL);

            SendToServer(fileMessage);
        }
    }

    //收到file message的回调函数，server收到的应该是result， client收到的应该是testcase
    void onFileMsg(const TcpConnectionPtr &conn, const RecvFilePtr& message, Timestamp time)
    {
        //文件发送结束
        if (message->file_type() == lkpMessage::File::END)
        {
            fclose(fp_);//必须关闭，不然会错误
            
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
            nodeID_ = message->node_id();
            fileName_ = "./testcase/node" + std::to_string(nodeID_) + "/client_testcase";
            printf("fileName_:%s\n",fileName_.c_str());

            fileSize_ = message->file_size();
            fp_ = ::fopen(fileName_.c_str(), "wb");
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
        //TODO : Check if lastPid_ is end. 
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
    int kBufSize_;
    pid_t lastPid_;
    string lastCmdString_;
};

int main(int argc, char *argv[])
{
    uint16_t port = 7777;
    int seconds = 3;
    string ip = "114.212.125.145";

    if (argc != 3)
    {
        printf("请输入：1.端口号 2.发送OK的间隔 \n");
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