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

#include "lib/lkpProto.pb.h"
#include "lib/lkpCodec.h"
#include "lib/lkpDispatcher.h"
#include "lkpHelper.h"

using namespace muduo;
using namespace muduo::net;

typedef std::shared_ptr<lkpMessage::Return> ReturnPtr;

class lkpCmdClient : boost::noncopyable
{
public:
    lkpCmdClient(EventLoop *loop, uint16_t port, lkpMessage::Command commandToSend)
        : loop_(loop),
          client_(loop, InetAddress("127.0.0.1", port), "lkpCmdClient"),
          commandToSend_(commandToSend),
          dispatcher_(bind(&lkpCmdClient::onUnknownMsg, this,
                           boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)),
          codec_(bind(&lkpDispatcher::onProtobufMessage, &dispatcher_,
                      boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3))
    {
        //绑定业务回调函数
        dispatcher_.registerMessageCallback<lkpMessage::Return>(bind(&lkpCmdClient::onReturnMsg,
                                                                     this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

        //绑定新连接请求回调函数
        client_.setConnectionCallback(
            bind(&lkpCmdClient::onConnection, this, boost::placeholders::_1));
        //绑定client的信息接收回调函数到lkpCodec
        client_.setMessageCallback(
            bind(&lkpCodec::onMessage, &codec_,
                 boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

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

    //向服务器发送数据
    void SendToServer(const lkpMessage::Command &messageToSend)
    {
        if (connection_->connected())
        {
            codec_.send(connection_, messageToSend);
        }
        else
        {
            printf("error to send!\n");
        }
    }

private:
    // lkpCmdClient类目前只支持1个连接
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {

            connection_ = conn;
            //只要连接上就立刻发命令
            SendToServer(commandToSend_);
        }
        else
        {
            // need do something?
        }
    }

    //收到Server的command运行结果return，打印return到terminal
    void onReturnMsg(const TcpConnectionPtr &conn, const ReturnPtr &message, Timestamp time)
    {
        lkpMessage::commandID myCommandEnum = message->command();
        string myCommandString;
        lkpEnumToCmds(myCommandEnum, myCommandString);
        printf("lkpCommand: Receive a return message, command type: %s!\n", myCommandString.c_str());

        //list
        if (myCommandEnum == lkpMessage::LIST)
        {
            uint32_t clientNum = message->client_num();
            printf("LIST: %u clients have connected..\n", clientNum);
            lkpMessage::Return::NodeInfo node;
            for (int i = 0; i < clientNum; ++i)
            {
                node = message->node_info(i);
                printf("   Node %2u: %s\n", node.node_id(), node.node_msg().c_str());
            }
        }
    }

    //收到未知数据包的回调函数
    void onUnknownMsg(const TcpConnectionPtr &conn, const MessagePtr &message, Timestamp time)
    {
        printf("Error!\n");
    }

    EventLoop *loop_;
    TcpClient client_;

    lkpDispatcher dispatcher_;
    lkpCodec codec_;

    MutexLock mutex_;
    TcpConnectionPtr connection_;

    lkpMessage::Command commandToSend_;
};

int main(int argc, char *argv[])
{
    EventLoop loop;

    const uint16_t port = 7777;
    lkpMessage::Command commandToSend;

    // argv style, argc = 4
    // TODO : 现在这里面无法区分testcase和testcluster，研究一下lkp里面的testcluster是什么东西
    // [RUN] [TESTCASE] [NODEID] [VMCNT]
    // [UPDATE/RESULT] [NULL] [NODEID] [NULL]
    // [PUSH] [TESTCASE]  [NODEID] [NULL]
    if (argc != 5)
    {
        printf("  Usage: [Command] [Testcase] [NodeID] [ContainerCnt]\n");
        return -1;
    }
    else
    {
        lkpMessage::commandID lkpEnum;

        //Command
        if (lkpCmdsToEnum(string(argv[1]), lkpEnum)){
            commandToSend.set_command(lkpEnum);
        }
            
        else{
            printf("lkpCmdClient Error: No supprt for command %s\n"
                   "  Usage: [Command] [Testcase] [NodeID] [ContainerCnt]\n",
                   argv[1]);
        }

        //TODO: should check if testcase name is valid

        //Testcase
        commandToSend.set_testcase(string(argv[2]));


        //NodeID，-1表示发送给全体
        int nodeID = atoi(argv[3]);
        if (nodeID >= 0){
            commandToSend.set_node_id(nodeID);
            commandToSend.set_send_to_all(false);
        }
        else{
            commandToSend.set_node_id(-1);
            commandToSend.set_send_to_all(true);
        }
            
        //ContainerCnt
        int dockerNum = atoi(argv[4]);
        if (dockerNum > 0){
            commandToSend.set_docker_num(dockerNum);
        }
            
    }

    lkpCmdClient client(&loop, port, commandToSend);
    client.connect();
    loop.loop();
}