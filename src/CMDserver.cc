#include "CMDserver.h"

CMDserver ::CMDserver(EventLoop *loop,
                      const InetAddress &listenAddr, int numThreads,int idleSeconds, int sfd)
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


    //定时断开无响应客户端的连接
    //time wheeling
    connectionBuckets_.resize(idleSeconds);
    loop->runEvery(1.0,std::bind(&CMDserver::onTimer,this));


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

    idleNodeID.clear();
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
        
        //没有闲置的nodeID
        int nodeID = -1;
        if(idleNodeID.empty()){
            nodeID = nodeCount++;
            connections_[nodeID] = conn; //新客户端加入

            printf("新客户端加入，nodeID is:%d\n",nodeID);
        }
        //分配最小的闲置nodeID
        else{
            nodeID = *idleNodeID.begin();
            idleNodeID.erase(idleNodeID.begin());
            connections_[nodeID] = conn; //新客户端加入

            printf("新客户端加入，nodeID is:%d\n",nodeID);
        }

        //time wheeling
        EntryPtr entry(new Entry(conn,this,nodeID));//为新客户端分配Entry
        connectionBuckets_.back().insert(entry);
        WeakEntryPtr weakEntry(entry); //弱引用是为了避免增加引用计数
        conn->setContext(weakEntry);   //把弱引用放入TcpConnectionPtr的setContext，从而可以取出
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

    //time wheeling
    // printf("收到客户端\n");

    WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext())); //利用Context取出弱引用
    EntryPtr entry(weakEntry.lock());                                          //引用一次，增加引用计数
    if (entry)
    {
        // printf("收到客户端的信息，nodeID is:%d\n",entry->Entry_nodeID);
        connectionBuckets_.back().insert(entry); //放入环形缓冲区，缓冲区的每个位置放置1个哈希表，哈系表的元素是shared_ptr<Entry>
    }


    StringPiece msg(buf->retrieveAllAsString());
    // LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at " << time.toString();
    // conn->send(msg);

    //TO DO :: 非阻塞

    //是客户端的连接数据
    if (strcmp(msg.data(),"OK") == 0)
    {
        // printf("收到客户端的OK，nodeID is:%d\n",entry->Entry_nodeID);
        return;
    }

    // 向CMDclient发送
    printf("收到客户端的数据：%s\n",msg.data());

    // char bufTemp[4096];
    // memcpy(bufTemp, msg, sizeof(bufTemp));
    int tmp = send(CMDcfd, msg.data(), msg.size(),0);
    if (tmp < 0)
    {
        perror("发送给CMDclient 失败\n");
    }
    else if(tmp == 0){
        close(CMDcfd);
    }
}

//计时器，前进tail
void CMDserver::onTimer()
{
    connectionBuckets_.push_back(Bucket()); //因为环形队列的大小已经固定，在队尾插入会导致删除
    // dumpConnectionBuckets();
}


//打印引用计数
void CMDserver::dumpConnectionBuckets() const
{
    int idx = 0;
    for (WeakConnectionList::const_iterator bucketI = connectionBuckets_.begin();
         bucketI != connectionBuckets_.end();
         ++bucketI, ++idx)
    {
        const Bucket &bucket = *bucketI;
        printf("[%d] len = %zd : ", idx, bucket.size());
        for (const auto &it : bucket)
        {
            bool connectionDead = it->weakConn_.expired();
            printf("%p(%ld)%s, ", get_pointer(it), it.use_count(),
                   connectionDead ? " DEAD" : "");
        }
        puts("");
    }
    printf("\n");
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

