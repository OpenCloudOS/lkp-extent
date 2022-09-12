#include "lkpServer.h"

lkpServer::lkpServer(EventLoop *loop,
                     const InetAddress &listenAddr, int numThreads, int idleSeconds,
                     off_t rollSize, int flushInterval)
    : server_(loop, listenAddr, "lkpServer"),
      loop_(loop),
      numThreads_(numThreads),
      /*lkpCodec & lkpDispatcher*/
      //绑定dispatcher_收到消息后的默认回调函数，这里设置为收到的message类型未知时的回调函数
      dispatcher_(std::bind(&lkpServer::onUnknownMsg, this,
                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
      //服务器收到消息后，解析形成对应的message，用message作为参数执行onProtobufMessage，哈系表已经存放了message类型对应的回调函数CallbackT
      codec_(std::bind(&lkpDispatcher::onProtobufMessage, &dispatcher_,
                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
    //push的文件
      kBufSize_(64 * 1024),

      /* 高速缓冲区使用变量，日志文件使用*/
      flushInterval_(flushInterval),
      running_(false),
      rollSize_(rollSize),
      thread_(std::bind(&lkpServer::threadFunc, this), "Logging_SendtoCMDclient"),
      latch_(1),
      cond_(mutex_),
      currentBuffer_(new Buffer_log),
      nextBuffer_(new Buffer_log),
      buffers_()

{
    //绑定业务lkpMessage::xxxxx的回调函数，lkpMessage::Command等在.proto文件中
    dispatcher_.registerMessageCallback<lkpMessage::Command>(std::bind(&lkpServer::onCommandMsg,
                                                                       this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    dispatcher_.registerMessageCallback<lkpMessage::CommandACK>(std::bind(&lkpServer::onCommandACK,
                                                                          this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    dispatcher_.registerMessageCallback<lkpMessage::PushACK>(std::bind(&lkpServer::onPushACK,
                                                                       this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    dispatcher_.registerMessageCallback<lkpMessage::File>(std::bind(&lkpServer::onFileMsg,
                                                                    this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    dispatcher_.registerMessageCallback<lkpMessage::HeartBeat>(std::bind(&lkpServer::onHeartBeat,
                                                                         this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //绑定新连接请求回调函数
    server_.setConnectionCallback(
        bind(&lkpServer::onConnection, this, boost::placeholders::_1));

    //绑定lkpCodec接收server新消息的回调函数，解析后形成正确类型的message
    server_.setMessageCallback(
        bind(&lkpCodec::onMessage, &codec_, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

    

    //定时断开无响应客户端的连接
    connectionBuckets_.resize(idleSeconds);
    loop->runEvery(1.0, std::bind(&lkpServer::onTimer, this));

    //设置IO线程的数量
    server_.setThreadNum(numThreads_);

    //缓冲区使用
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

//启动服务器
void lkpServer ::start()
{
    server_.start();

    //push的文件描述符
    fpMap.clear();

    //logging
    running_ = true; //允许后端写日志
    thread_.start(); //启动后端线程threadFunc
    latch_.wait();   //等待后端线程threadFunc启动，否则服务器不能执行其他动作
}

//向客户端发送数据
void lkpServer ::SendToClient(const google::protobuf::Message &message, const TcpConnectionPtr& conn)
{
    if (conn && conn->connected())
    {
        codec_.send(conn, message);
    }
    else
        printf("No such client!\n");
}

//向命令行回复结果
void lkpServer ::SendToCmdClient(const google::protobuf::Message &message)
{
    if (CmdConnection_->connected())
    {
        codec_.send(CmdConnection_, message);
    }
}

//客户端请求建立新的连接
void lkpServer ::onConnection(const TcpConnectionPtr &conn)
{
    // LOG_INFO << conn->peerAddress().toIpPort() << " -> "
    //          << conn->localAddress().toIpPort() << " is "
    //          << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
        //如果是来自本地回环地址的连接，应该是唯一的命令行
        if (!conn->peerAddress().toIp().compare("127.0.0.1"))
        {
            if (!hasCmdConnected_)
            {
                CmdConnection_ = conn;
                printf("lkpServer: Has connected to cmdClient!\n");
            }
            else
            {
                CmdConnection_->shutdown();
                printf("lkpServer Error: Has connected to a CmdClient!\n");
            }
        }
        //和客户端建立连接
        else
        {
            int nodeID = clientPool_.add(conn);
            // printf("onConnection nodeID is:%d\n",nodeID);

            //time wheeling
            EntryPtr entry(new Entry(conn, this, nodeID)); //为新客户端分配Entry
            connectionBuckets_.back().insert(entry);
            WeakEntryPtr weakEntry(entry); //弱引用是为了避免增加引用计数
            conn->setContext(weakEntry);   //把弱引用放入TcpConnectionPtr的setContext，从而可以取出
        }
    }
    else
    {
        // LocalConnections::instance().erase(conn);
    }
}

void lkpServer::pushToClient(const RecvCommandPtr &message)
{
    string fileName = message->testcase(); //文件名称
    //获取文件的大小
    struct stat statbuf;
    stat(fileName.c_str(), &statbuf);
    int fileSize = statbuf.st_size;

    //单播
    if (!message->send_to_all())
    {
        clientNum_ = 1;

        FILE *fp = ::fopen(fileName.c_str(), "rb"); //打开文件
        if (!fp)
        {
            perror("open file fail!!\n");
            return;
        }

        int nodeID = message->node_id();
        TcpConnectionPtr conn = clientPool_.getConn(nodeID);

        FilePtr ctx(fp, ::fclose); //多了::fclose参数，表示fp对象的引用计数器变成0时，调用::fclose来销毁fp
        fpMap[conn] = ctx;
        char buf[kBufSize_];
        size_t nread = ::fread(buf, 1, sizeof buf, fp); //读取kBufSize的内容

        lkpMessage::File fileMessage;
        fileMessage.set_file_type(lkpMessage::File::TESTCASE);
        fileMessage.set_node_id(nodeID);
        fileMessage.set_file_size(fileSize);
        fileMessage.set_patch_len(nread);
        fileMessage.set_first_patch(true);
        fileMessage.set_content(buf);

        conn->setWriteCompleteCallback(bind(&lkpServer::onWriteComplete, this, boost::placeholders::_1)); //发完一次后继续发
        SendToClient(fileMessage, conn);
    }
    //广播
    else
    {
        clientNum_ = clientPool_.size();

        //对所有节点作一次单播
        for (auto it : clientPool_.connections_)
        {
            FILE *fp = ::fopen(fileName.c_str(), "rb"); //打开文件
            if (!fp)
            {
                perror("open file fail!!\n");
                return;
            }

            int nodeID = it.first;
            TcpConnectionPtr conn = it.second;

            FilePtr ctx(fp, ::fclose); //多了::fclose参数，表示fp对象的引用计数器变成0时，调用::fclose来销毁fp
            fpMap[conn] = ctx;
            char buf[kBufSize_];
            size_t nread = ::fread(buf, 1, sizeof buf, fp); //读取kBufSize的内容

            lkpMessage::File fileMessage;
            fileMessage.set_file_type(lkpMessage::File::TESTCASE);
            fileMessage.set_node_id(nodeID);
            fileMessage.set_file_size(fileSize);
            fileMessage.set_patch_len(nread);
            fileMessage.set_first_patch(true);
            fileMessage.set_content(buf);

            conn->setWriteCompleteCallback(bind(&lkpServer::onWriteComplete, this, boost::placeholders::_1)); //发完一次后继续发
            SendToClient(fileMessage, conn);
        }
    }
}


void  lkpServer::BroadToClients(lkpMessage::Command message){
    for (auto it = clientPool_.connections_.begin(); it != clientPool_.connections_.end(); ++it){
        message.set_node_id(it->first);
        SendToClient(message, it->second);
    }
}

//收到命令的回调函数，server转发给client， client执行。并且回复命令行
void lkpServer ::onCommandMsg(const TcpConnectionPtr &conn, const RecvCommandPtr &message, Timestamp time)
{
    clientNum_ = clientOKNum_ = ackTimes_ = 0;
    clientPool_.clear_info();

    //解析命令行的命令
    lkpMessage::commandID myCommand = message->command();
    string myCommandString;
    lkpEnumToCmds(myCommand, myCommandString);
    printf("Recv a command: %s\n", myCommandString.c_str());
    
    switch(myCommand){
        case lkpMessage::UPDATE:
        case lkpMessage::RUN:
        case lkpMessage::RESULT:{
            if (!message->send_to_all() && message->node_id()>=0){
                SendToClient(*message, clientPool_.getConn(message->node_id()));
                clientNum_ = 1;
            }
            else{    
                BroadToClients(*message);
                clientNum_ = clientPool_.size();
            }
            break;
        }
        case lkpMessage::PUSH:{
            pushToClient(message);
        }
        case lkpMessage::LIST:{
            lkpMessage::Return ReturnToSend;//返回给命令行的回复
            ReturnToSend.set_command(myCommand);
            ReturnToSend.set_client_num(clientPool_.size());
            ReturnToSend.set_client_ok_num(clientPool_.size());

            //所有在线客户端的信息
            lkpMessage::Return::NodeInfo *NodeInfoPtr;
            for (auto it = clientPool_.connections_.begin(); it != clientPool_.connections_.end(); ++it)
            {
                NodeInfoPtr = ReturnToSend.add_node_info();
                NodeInfoPtr->set_node_id(it->first);
                NodeInfoPtr->set_node_msg(it->second->peerAddress().toIp());
            }

            //回复给命令行
            SendToCmdClient(ReturnToSend);
            break;
        }
    }

}

//每次发送64kb
void lkpServer ::onWriteComplete(const TcpConnectionPtr &conn)
{
    FilePtr &fp = fpMap[conn];

    char buf[kBufSize_];
    size_t nread = ::fread(buf, 1, sizeof(buf), get_pointer(fp));

    //续传
    if (nread > 0)
    {
        lkpMessage::File fileMessage;
        fileMessage.set_file_type(lkpMessage::File::TESTCASE);
        fileMessage.set_first_patch(false);
        fileMessage.set_patch_len(nread);
        fileMessage.set_content(buf);

        SendToClient(fileMessage,conn);
    }
    //结束
    else
    {
        //发送结束信号
        lkpMessage::File fileMessage;
        fileMessage.set_file_type(lkpMessage::File::END);

        conn->setWriteCompleteCallback(NULL);

        SendToClient(fileMessage,conn);

        printf("push testcase to client end!\n");

    }
}

//收到pushack的回调函数，应该开始发testecase的文件内容
void lkpServer ::onPushACK(const TcpConnectionPtr &conn, const PushACKPtr &message, Timestamp time)
{

    ackTimes_++;
    if(message->status()){
        clientOKNum_++;
    }

    //收到错误才回复给命令行
    if(!message->status()){
        clientPool_.update_info(message->node_id(),message->ack_message());
    }

    //回复给命令行
    if(ackTimes_ == clientNum_){
        lkpMessage::Return Return;
        Return.set_command(lkpMessage::commandID::PUSH);
        Return.set_client_num(clientNum_);
        Return.set_client_ok_num(clientOKNum_);

        //出错的节点记录在info
        for (auto it = clientPool_.node_info.begin(); it != clientPool_.node_info.end(); ++it){
            lkpMessage::Return::NodeInfo *NodeInfoPtr = Return.add_node_info();
            NodeInfoPtr->set_node_id(it->first);
            NodeInfoPtr->set_node_msg(it->second);
        }   

        SendToCmdClient(Return);
    }
}

//收到command ACK的回调函数，应该使统计数量++
void lkpServer ::onCommandACK(const TcpConnectionPtr &conn, const CommandACKPtr &message, Timestamp time)
{
    ackTimes_++;
    if(message->status()){
        clientOKNum_++;
    }

    //收到错误才回复给命令行
    if(!message->status()){
        clientPool_.update_info(message->node_id(),message->ack_message());
    }

    //回复给命令行
    if(ackTimes_ == clientNum_){
        lkpMessage::Return Return;
        Return.set_command(message->command());
        Return.set_client_num(clientNum_);
        Return.set_client_ok_num(clientOKNum_);

        //出错的节点记录在info
        for (auto it = clientPool_.node_info.begin(); it != clientPool_.node_info.end(); ++it){
            lkpMessage::Return::NodeInfo *NodeInfoPtr = Return.add_node_info();
            NodeInfoPtr->set_node_id(it->first);
            NodeInfoPtr->set_node_msg(it->second);
        }   

        SendToCmdClient(Return);
    }
}

//收到file message的回调函数，server收到的应该是result， client收到的应该是testcase
void lkpServer ::onFileMsg(const TcpConnectionPtr &conn, const RecvFilePtr &message, Timestamp time)
{
    //文件发送结束
    if (message->file_type() == lkpMessage::File::END)
    {
        ackTimes_++;

        ::fclose(outputfpMap_[conn]); //必须关闭，不然会错误

        //获取文件的大小
        struct stat statbuf;
        stat(fileNameMap_[conn].c_str(), &statbuf);
        int recvSize = statbuf.st_size;

        //检查文件是否完整
        if (recvSize != fileSizeMap_[conn])
        {
            printf("recvSize:%d,fileSize_:%d\n", recvSize, fileSizeMap_[conn]);
            printf("node:%d file is not complete!\n",message->node_id());

            clientPool_.update_info(message->node_id(),"file is not completed");
        }
        else
        {
            printf("recv a complete file\n");

            //成功
            clientOKNum_++;
        }

        //回复给命令行
        if (ackTimes_ == clientNum_)
        {
            lkpMessage::Return Return;
            Return.set_command(lkpMessage::commandID::RESULT);
            Return.set_client_num(clientNum_);
            Return.set_client_ok_num(clientOKNum_);

            //出错的节点记录在info
            for (auto it = clientPool_.node_info.begin(); it != clientPool_.node_info.end(); ++it)
            {
                lkpMessage::Return::NodeInfo *NodeInfoPtr = Return.add_node_info();
                NodeInfoPtr->set_node_id(it->first);
                NodeInfoPtr->set_node_msg(it->second);
            }

            SendToCmdClient(Return);
        }

        return;
    }
    //第一次接收
    else if (message->first_patch())
    {
        int nodeID = stoi(message->file_name());
        fileNameMap_[conn] = "./testcase/server/result" + std::to_string(nodeID);
        printf("fileName_:%s\n", fileNameMap_[conn].c_str());

        fileSizeMap_[conn] = message->file_size();
        FILE *fp = ::fopen(fileNameMap_[conn].c_str(), "wb");
        if (!fp)
        {
            perror("open file fail!!\n");
            return;
        }
        outputfpMap_[conn] = fp;
    }

    //每次接收的都输出
    fwrite(message->content().c_str(), 1, message->patch_len(), outputfpMap_[conn]);
}

//收到心跳包的回调函数
void lkpServer ::onHeartBeat(const TcpConnectionPtr &conn, const HeartBeatPtr &message, Timestamp time)
{
    //printf("recv a HeartBeat, status is %d\n", (int)message->status());
    //time wheeling
    WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext())); //利用Context取出弱引用

    printf("Entry_nodeID:%d\n",(weakEntry.lock())->Entry_nodeID);

    EntryPtr entry(weakEntry.lock());                                          //引用一次，增加引用计数
    if (entry)
    {
        // printf("收到客户端的信息，nodeID is:%d\n",entry->Entry_nodeID);
        connectionBuckets_.back().insert(entry); //放入环形缓冲区，缓冲区的每个位置放置1个哈希表，哈系表的元素是shared_ptr<Entry>
    }
}

//收到未知数据包的回调函数
void lkpServer ::onUnknownMsg(const TcpConnectionPtr &conn, const MessagePtr &message, Timestamp time)
{
    printf("error! shut down the connection\n");
}

//计时器，前进tail
void lkpServer::onTimer()
{
    connectionBuckets_.push_back(Bucket()); //因为环形队列的大小已经固定，在队尾插入会导致删除
    // dumpConnectionBuckets();
}

//打印心跳连接情况
void lkpServer::dumpConnectionBuckets() const
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

//onMessage调用，负责向一级缓冲区写
void lkpServer::append(const char *logline, int len)
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

//后端调用，向日志文件写 muduo::Thread thread_ 绑定该函数，实现后端线程负责写
void lkpServer::threadFunc()
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
