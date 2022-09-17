#include "lkpClient.h"

lkpClient::lkpClient(EventLoop *loop, const InetAddress &serverAddr, int seconds)
    : loop_(loop),
      client_(loop, serverAddr, "lkpClient"),
      seconds_(seconds),
      kBufSize_(64 * 1024),
      dispatcher_(std::bind(&lkpClient::onUnknownMsg, this,
                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
      codec_(std::bind(&lkpDispatcher::onProtobufMessage, &dispatcher_,
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

      /* 高速缓冲区使用变量，日志文件使用*/
      connection_(nullptr),
      flushInterval_(3),
      running_(false),
      rollSize_(500 * 1000 * 1000),
      thread_(std::bind(&lkpClient::threadFunc, this), "Logging_SendtoCMDclient"),
      latch_(1),
      cond_(mutex_),
      currentBuffer_(new Buffer_log),
      nextBuffer_(new Buffer_log),
      buffers_(),
      basename_(ROOT_DIR + "/log/clientfile") //缓冲区的文件名称
{
    //缓冲区使用
    
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);

    //logging
    running_ = true; //允许后端写日志
    thread_.start(); //启动后端线程threadFunc
    latch_.wait();   //等待后端线程threadFunc启动，否则服务器不能执行其他动作

    //绑定业务回调函数
    dispatcher_.registerMessageCallback<lkpMessage::Command>(std::bind(&lkpClient::onCommandMsg,
                                                                  this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    dispatcher_.registerMessageCallback<lkpMessage::File>(std::bind(&lkpClient::onFileMsg,
                                                               this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //绑定新连接请求回调函数
    client_.setConnectionCallback(
        std::bind(&lkpClient::onConnection, this, std::placeholders::_1));
    //绑定client的信息接收回调函数到lkpCodec
    client_.setMessageCallback(
        std::bind(&lkpCodec::onMessage, &codec_,
             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //绑定定时器产生心跳包
    loop_->runEvery(seconds_, std::bind(&lkpClient::onTimer, this));
}

lkpClient::~lkpClient()
{
    if (running_)
    {
        running_ = false; //不允许后端继续写日志
        cond_.notify();   //避免后端卡在条件变量处
        thread_.join();   //回收后端线程
    }
}

//连接服务器
void lkpClient::connect()
{
    client_.connect();
}

//断开与服务器的连接
void lkpClient::disconnect()
{
    client_.disconnect();
}


// 该函数在IO线程中执行，IO线程与主线程不在同一个线程
void lkpClient::onConnection(const TcpConnectionPtr &conn)
{
    // // mutex用来保护connection_这个shared_ptr
    // MutexLockGuard lock(mutex_);
    if (conn->connected())
    {
        connection_ = conn;
        std::cout << "lkp-extent client init success " << std::endl;
        if(!daemon(1,0));
            std::cout << "lkp-extent error: cannot run as daemon!" << std::endl;
    }
    else
    {
        connection_.reset();
        connection_ = nullptr;
        LOG_INFO << "lkp-ctl client unconnected !!!";
    }
}

//收到命令的回调函数，server转发给client， client执行
void lkpClient::onCommandMsg(const TcpConnectionPtr &conn, const RecvCommandPtr &message, Timestamp time)
{
    lkpMessage::CommandACK ACK;
    ACK.set_command(message->command());
    ACK.set_node_id(message->node_id());
    int status = 0;

    //如果上一个命令还没有运行完，现场尝试回收子进程，如果回收失败，说明上一条命令此时还不可能结束
    if (lastPid_ > 0 && waitpid(lastPid_, &status, WNOHANG) == 0)
    {
        lastStatus_ = WIFEXITED(status);
        ACK.set_status(false);
        ACK.set_ack_message("ERROR 10: Command " + lastCmdString_ + " is still ruunning!");
        SendToServer(ACK);
        return;
    }

    lkpEnumToCmds(message->command(), lastCmdString_);
    lastPid_ = 0;
    switch (message->command())
    {

    case lkpMessage::UPDATE:
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            ACK.set_status(false);
            ACK.set_ack_message("ERROR 11: Fork Error!");
            SendToServer(ACK);
            return;
        }
        //开启新进程执行命令
        if (pid == 0)
        {
            if (execlp("lkp-ctl", "lkp-ctl", "update", NULL) < 0)
            {
                perror("Error on UPDATE exec:");
                ACK.set_status(false);
                ACK.set_ack_message("ERROR 12: Command UPDATE cannot run!");
                SendToServer(ACK);
                exit(0);
            }
        }
        else
        {
            lastPid_ = pid;
            if (waitpid(pid, &status, WNOHANG) == -1)
            {
                ACK.set_status(false);
                ACK.set_ack_message("ERROR 12: Command UPDATE cannot run!");
                SendToServer(ACK);
            }
            else
            {
                lastStatus_ = WIFEXITED(status);
                ACK.set_status(true);
                SendToServer(ACK);
            }
        }
        break;
    }

    case lkpMessage::RUN:
    {
        pid_t pid;
        string testname = message->testcase();
        unsigned int dockerNum = message->docker_num();
        string dockerNumString = std::to_string(dockerNum);
        pid = fork();
        if (pid == 0)
        {
            //子进程执行lkp-ctl run (-c vm_cnt) testcase
            if (message->docker_num())
            {
                if (execlp("lkp-ctl", "lkp-ctl", "-c", dockerNumString.data(), "run", testname.data(), NULL) < 0)
                {
                    perror("Error on RUN exec:");
                    exit(0);
                }
            }
            else
            {
                if (execlp("lkp-ctl", "lkp-ctl", "run", testname.data(), NULL) < 0)
                {
                    perror("Error on RUN exec:");
                    exit(0);
                }
            }
        }
        else if (pid < 0)
        {
            ACK.set_status(false);
            ACK.set_ack_message("ERROR 11: Fork Error!");
            SendToServer(ACK);
            return;
        }
        //开启新进程执行命令
        else
        {
            lastPid_ = pid;
            if (waitpid(pid, &status, WNOHANG) == -1)
            {
                ACK.set_status(false);
                ACK.set_ack_message("ERROR 12: Command RUN cannot run!");
                SendToServer(ACK);
            }
            else
            {
                lastStatus_ = WIFEXITED(status);
                ACK.set_status(true);
                SendToServer(ACK);
            }
        }
        break;
    }

    case lkpMessage::RESULT:
    {
        onResult(conn, message);
        ACK.set_status(true);
        SendToServer(ACK);
        break;
    }

    case lkpMessage::PUSH:
    {
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
void lkpClient::onResult(const TcpConnectionPtr &conn, const RecvCommandPtr &message)
{
    nodeID_ = message->node_id();

    string fileName = ROOT_DIR + "/results/local/result.tar";

    LOG_INFO << "Result fileName:" << fileName;

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
    conn->setContext(ctx);     //把TcpConnectionPtr对象与fp绑定
    char buf[kBufSize_];
    size_t nread = ::fread(buf, 1, sizeof buf, fp); //读取kBufSize的内容

    lkpMessage::File fileMessage;
    fileMessage.set_file_type(lkpMessage::File::RESULT);
    fileMessage.set_file_name(std::to_string(nodeID_));
    fileMessage.set_file_size(fileSize);
    fileMessage.set_patch_len(nread);
    fileMessage.set_first_patch(true);
    fileMessage.set_content(buf);
    fileMessage.set_file_name("result.tar");

    conn->setWriteCompleteCallback(bind(&lkpClient::onWriteComplete, this, std::placeholders::_1)); //发完一次后继续发
    SendToServer(fileMessage);
}

//每次发送64kb
void lkpClient::onWriteComplete(const TcpConnectionPtr &conn)
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
void lkpClient::onFileMsg(const TcpConnectionPtr &conn, const RecvFilePtr &message, Timestamp time)
{
    //文件发送结束
    if (message->file_type() == lkpMessage::File::END)
    {
        fclose(fp_); //必须关闭，不然会错误

        //获取文件的大小
        struct stat statbuf;
        stat(fileName_.c_str(), &statbuf);
        int recvSize = statbuf.st_size;

        //检查文件是否完整
        if (recvSize != fileSize_)
        {
            // printf("recvSize:%d,fileSize_:%d\n", recvSize, fileSize_);
            // printf("file is not complete!\n");
            LOG_INFO << "recvSize:" << recvSize << ",fileSize_:" << fileSize_;
            LOG_INFO << "file is not complete!";

            //失败回复
            lkpMessage::PushACK ack;
            ack.set_status(false);
            ack.set_ack_message("push recv fail");
            ack.set_node_id(nodeID_);
            SendToServer(ack);
            return;
        }
        else
        {
            // printf("recv a complete file\n");
            LOG_INFO << "recv a complete file";

            //成功
            lkpMessage::PushACK ack;
            ack.set_status(true);
            ack.set_ack_message("push recv success");
            ack.set_node_id(nodeID_);
            SendToServer(ack);
            const string copyToJobs = "cp " + fileName_ + " " + ROOT_DIR + "/lkp-tests/jobs";
            system(copyToJobs.c_str());
            return;
        }
    }
    //第一次接收
    else if (message->first_patch())
    {
        nodeID_ = message->node_id();
        fileName_ = ROOT_DIR + "/testcases/" + message->file_name();
        LOG_INFO << "fileName_:" << fileName_;
        
        fileSize_ = message->file_size();
        fp_ = ::fopen(fileName_.c_str(), "wb");
        assert(fp_);
    }

    //每次接收的都输出
    fwrite(message->content().c_str(), 1, message->patch_len(), fp_);
}

//收到未知数据包的回调函数
void lkpClient::onUnknownMsg(const TcpConnectionPtr &conn, const MessagePtr &message, Timestamp time)
{
    LOG_INFO << "Error: Unknown Message, shut dowm the connect!";
    conn->shutdown();
    exit(0);
}

//定期心跳回调函数
void lkpClient::onTimer()
{
    if(!connection_){
        printf("lkp-ctl client error: Cannot connect!\n");
        exit(0);
    }
    lkpMessage::HeartBeat heart;
    heart.set_status(true);
    SendToServer(heart);
    int status;
    if (lastPid_ > 0 && waitpid(lastPid_, &status, WNOHANG) != 0)
    {
        lastPid_ = 0;
    }
    lastStatus_ = WIFEXITED(status);
}

//向服务器发送数据
void lkpClient::SendToServer(const google::protobuf::Message &messageToSend)
{
    // // mutex用来保护connection_这个shared_ptr
    // MutexLockGuard lock(mutex_);
    if (connection_->connected())
    {
        codec_.send(connection_, messageToSend);
    }
}

//onMessage调用，负责向一级缓冲区写
void lkpClient::append(const char *logline, int len)
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
void lkpClient::threadFunc()
{
    assert(running_ == true);

    //让start()的latch_.wait()得到0，可以运行前端线程
    latch_.countDown();

    LogFile output(basename_, rollSize_, false);

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
            output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end()); //丢弃多余的日志，只保留前2个
        }

        for (const auto &buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length()); //写日志
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
        output.flush();
    }
}

lkpClient *g_asyncLog_client = NULL;
//前端写日志时调用
void asyncOutput_client(const char *msg, int filelen)
{
    g_asyncLog_client->append(msg, filelen);
}
