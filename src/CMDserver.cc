#include "CMDserver.h"

CMDserver ::CMDserver(EventLoop *loop,
                      const InetAddress &listenAddr, int numThreads, int idleSeconds, int sfd,
                      off_t rollSize, int flushInterval)
    : server_(loop, listenAddr, "CMDserver"),
      loop_(loop),
      numThreads_(numThreads),
      CMDserverSfd(sfd),
      nodeCount(0),
      flushInterval_(flushInterval),
      running_(false),
      rollSize_(rollSize),
      thread_(std::bind(&CMDserver::threadFunc, this), "Logging_SendtoCMDclient"),
      latch_(1),
      cond_(mutex_),
      currentBuffer_(new Buffer_log),
      nextBuffer_(new Buffer_log),
      buffers_()
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
    loop->runEvery(1.0, std::bind(&CMDserver::onTimer, this));

    //设置IO线程的数量
    server_.setThreadNum(numThreads_);

    //logging
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

//logging
//onMessage调用，负责向一级缓冲区写
void CMDserver::append(const char *logline, int len)
{
    muduo::MutexLockGuard lock(mutex_); //加锁访问一级缓冲区

    //如果一级当前未满，直接加入
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    //一级当前已满，把一级当前加入二级列表，把一级预备转正
    else
    {
        buffers_.push_back(std::move(currentBuffer_)); //一级当前加入二级列表

        if (nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_); //一级预备转正
        }
        else
        {
            currentBuffer_.reset(new Buffer_log); //基本上不会发生，预备空时，重新申请
        }

        currentBuffer_->append(logline, len); //向一级当前写
        cond_.notify();                       //生产者释放条件变量，通知后端条件满足
    }
}
//logging
//后端调用，向日志文件写 muduo::Thread thread_ 绑定该函数，实现后端线程负责写
void CMDserver::threadFunc()
{
    assert(running_ == true);

    //让start()的latch_.wait()得到0，可以运行前端线程
    latch_.countDown();

    //准备2块缓冲区，用于分配给一级
    BufferPtr newBuffer1(new Buffer_log);
    BufferPtr newBuffer2(new Buffer_log);
    newBuffer1->bzero();
    newBuffer2->bzero();

    //后端操作的二级
    BufferVector buffersToWrite; //写日志操作的缓冲区
    buffersToWrite.reserve(16);  //预分配16个位置

    //真正运行的部分
    while (running_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        //加锁，此时前端不能操作一级
        {
            muduo::MutexLockGuard lock(mutex_);
            //条件变量，如果二级列表是空的，就等待超时、前端释放条件变量。
            if (buffers_.empty())
            {
                cond_.waitForSeconds(flushInterval_);
            }

            buffers_.push_back(std::move(currentBuffer_)); //未满的一级加入二级列表
            currentBuffer_ = std::move(newBuffer1);        //一级当前分配空间
            buffersToWrite.swap(buffers_);                 //交换，快速释放对一级的占用
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        //如果消息堆积，只保留前2块缓冲区
        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toFormattedString().c_str(),
                     buffersToWrite.size() - 2);
            fputs(buf, stderr);
            // output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end()); //丢弃多余的日志，只保留前2个
        }

        //向CMDclient发送
        for (const auto &buffer : buffersToWrite)
        {
            if (buffer->length() != 0)
            {
                int tmp = send(CMDcfd, buffer->data(), buffer->length(), 0);
                if (tmp < 0)
                {
                    perror("向CMDclient发送失败\n");
                }
            }
        }

        if (buffersToWrite.size() > 2)
        {
            //丢弃无用的缓冲区
            buffersToWrite.resize(2); //buffersToWrite的内容已经完成了写入，空闲了。只保留二级缓冲区列表的2个缓冲区，表示newbuffer1,newbuffer2
        }

        //为newBuffer1,newBuffer2分配空间
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        //newBuffer2可能分配给了一级缓冲区的预备
        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
    }
}

void CMDserver ::start()
{
    server_.start();

    //监听CMDclient
    Channel *ListenCMDclient = new Channel(loop_, CMDserverSfd);
    ListenCMDclient->setReadCallback(std::bind(&CMDserver::on_accept_CMD_IPC, this));
    ListenCMDclient->enableReading();

    idleNodeID.clear();

    //logging
    running_ = true; //允许后端写日志
    thread_.start(); //启动后端线程threadFunc
    latch_.wait();   //等待后端线程threadFunc启动，否则服务器不能执行其他动作
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
        if (idleNodeID.empty())
        {
            nodeID = nodeCount++;
            connections_[nodeID] = conn; //新客户端加入

            printf("新客户端加入，nodeID is:%d\n", nodeID);
        }
        //分配最小的闲置nodeID
        else
        {
            nodeID = *idleNodeID.begin();
            idleNodeID.erase(idleNodeID.begin());
            connections_[nodeID] = conn; //新客户端加入

            printf("新客户端加入，nodeID is:%d\n", nodeID);
        }

        //time wheeling
        EntryPtr entry(new Entry(conn, this, nodeID)); //为新客户端分配Entry
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
    if (strcmp(msg.data(), "OK") == 0)
    {
        // printf("收到客户端的OK，nodeID is:%d\n",entry->Entry_nodeID);
        return;
    }


    // 向CMDclient发送
    printf("收到客户端的数据：%s\n", msg.data());
    append(msg.data(),msg.size());

    // int tmp = send(CMDcfd, msg.data(), msg.size(), 0);
    // if (tmp < 0)
    // {
    //     perror("发送给CMDclient 失败\n");
    // }
    // else if (tmp == 0)
    // {
    //     close(CMDcfd);
    // }
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
void CMDserver ::on_accept_CMD_IPC()
{
    printf("尝试建立进程间通信的连接\n");
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    CMDcfd = accept(CMDsfd, (struct sockaddr *)&client, &len);

    printf("IPC建立， CMDcfd：%d\n", CMDcfd);

    //监听CMDcfd传过来的数据
    Channel *CMDcfdEv = new Channel(loop_, CMDcfd);
    CMDcfdEv->setReadCallback(std::bind(&CMDserver::recv_CMDclient, this, CMDcfd));
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
    else if (tmp == 0)
    {
        printf("CMDclient关闭连接\n");
        close(CMDcfd);
        return;
    }

    //向TCP客户端推送testcase
    //TO DO:每个线程连接一个客户端
    int nodeID = std::stoi(recvbuf);
    printf("收到CMDclient的数据 nodeID:%d\n", nodeID);

    if(nodeID >= nodeCount || idleNodeID.count(nodeID)){
        printf("服务器没有该客户端\n");
        char*tempch = new char[strlen("服务器没有该客户端") + 1];
        strcpy(tempch,"服务器没有该客户端");
        append(tempch,strlen("服务器没有该客户端") + 1);
        return;
    }

    //TO DO：TCP server 负责进行发送
    std::string stemp = "testcase";
    sendToTcpClient(stemp, nodeID);
}
