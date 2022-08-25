#include "CMDserver.h"

CMDserver ::CMDserver(EventLoop *loop,
                      const InetAddress &listenAddr, int numThreads, int sfd)
    : server_(loop, listenAddr, "CMDserver"),
      loop_(loop),
      numThreads_(numThreads),
      CMDserverSfd(sfd),
      nodeCount(0)
{
    //客户端请求建立新的连接
    server_.setConnectionCallback(
        bind(&CMDserver::onConnection, this, boost::placeholders::_1));

    //接收客户端的数据
    server_.setMessageCallback(
        bind(&CMDserver::onMessage, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

    //设置IO线程的数量
    server_.setThreadNum(numThreads_);
}

void CMDserver ::start()
{
    server_.start();

    //监听CMDclient
    Channel *ListenCMDclient = new Channel(loop_, CMDserverSfd);
    ListenCMDclient->setReadCallback(std::bind(&CMDserver::on_accept_CMD_IPC,this));
    ListenCMDclient->enableReading();
}

//向客户端发送数据
void CMDserver ::sendToTcpClient(const StringPiece &message, int nodeID)
{
    //向nodeID发送数据
    if (connections_[nodeID])
    {
        printf("向客户端发送数据：%s\n", message.data());
        connections_[nodeID]->send(message);
    }
}

//客户端请求建立新的连接
void CMDserver ::onConnection(const TcpConnectionPtr &conn)
{
    // LOG_INFO << conn->peerAddress().toIpPort() << " -> "
    //          << conn->localAddress().toIpPort() << " is "
    //          << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
        printf("新客户端加入\n");
        connections_[nodeCount++] = conn; //新客户端加入
    }
    else
    {
        // LocalConnections::instance().erase(conn);
    }
}

//接收客户端的数据，发送给CMDclient
void CMDserver::onMessage(const TcpConnectionPtr &conn,
                          Buffer *buf,
                          Timestamp time)
{
    StringPiece msg(buf->retrieveAllAsString());
    // LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at " << time.toString();
    // conn->send(msg);

    //向CMDclient发送
    // printf("收到客户端的数据：%s\n",msg);
    // char bufTemp[4096];
    // memcpy(bufTemp, msg.c_str(), sizeof(bufTemp));
    // if (send(CMDcfd, bufTemp, sizeof(bufTemp), 0) < 0)
    // {
    //     perror("发送给CMDclient 失败\n");
    // }

    printf("收到客户端的数据：%s\n",msg.data());
}

//建立进程通信的套接字
int buildIPC()
{
    //和命令行通信的套接字 server
    CMDsfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (CMDsfd < 0)
    {
        perror("socket fail");
        return 0;
    }
    struct sockaddr_un server;
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, CMDIPC);
    unlink(CMDIPC);
    int ret = bind(CMDsfd, (struct sockaddr *)&server, sizeof(server));
    if (ret < 0)
    {
        perror("bind fail");
        close(CMDsfd);
        unlink(CMDIPC);
        return 0;
    }
    ret = listen(CMDsfd, 300);
    if (ret < 0)
    {
        perror("listen CMDsfd fail");
        close(CMDsfd);
        unlink(CMDIPC);
        return 0;
    }
    printf("CMDserver sfd 建立成功");
    return 0;
}

//建立进程间的连接
void CMDserver :: on_accept_CMD_IPC()
{
    printf("尝试建立进程间通信的连接\n");
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    CMDcfd = accept(CMDsfd, (struct sockaddr *)&client, &len);

    printf("IPC建立， CMDcfd：%d\n", CMDcfd);


    //监听CMDcfd传过来的数据
    Channel *CMDcfdEv = new Channel(loop_, CMDcfd);
    CMDcfdEv->setReadCallback(std::bind(&CMDserver::recv_CMDclient, this,CMDcfd));
    CMDcfdEv->enableReading();
}

//接收CMDclient的数据
void CMDserver::recv_CMDclient(int CMDcfd)
{
    char recvbuf[LEN];

    //TO DO:接收CMD的输入
    int tmp = recv(CMDcfd, recvbuf, LEN, 0);
    if (tmp < 0)
    {
        perror("error");
        return;
    }
    else if(tmp == 0){
        printf("CMDclient关闭连接\n");
        close(CMDcfd);
        return;
    }


    //向TCP客户端推送testcase
    //TO DO:每个线程连接一个客户端
    int nodeID = std::stoi(recvbuf);
    printf("收到CMDclient的数据 nodeID:%d\n", nodeID);

    //TO DO：TCP server 负责进行发送
    std::string stemp = "testcase";
    sendToTcpClient(stemp,nodeID);
}

