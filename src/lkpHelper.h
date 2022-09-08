#ifndef LKP_HELPER
#define LKP_HELPER


#include "muduo/net/Buffer.h"
#include "muduo/net/TcpConnection.h"
#include "lib/lkpProto.pb.h"

// using namespace std;
using namespace muduo;
using namespace muduo::net;

extern std::vector<string> lkpCommands;

//将lkpCommand字符串转换为message中使用的enum
bool lkpCmdsToEnum(const string& lkpCmdString, lkpMessage::commandID& lkpEnum);
//将lkpMessage::commandID中的enum转换为字符串
bool lkpEnumToCmds(const lkpMessage::commandID lkpEnum, string& lkpCmdString);


class lkpClientPool : noncopyable
{
    friend class lkpServer;
public:
    lkpClientPool();

    //客户端总量
    int size();

    //不是命令行时才调用，增加新客户端
    int add(TcpConnectionPtr conn);

    //删除ID
    bool del(int nodeID);

    //获取conn
    TcpConnectionPtr getConn(int nodeID);
private:
    //client pool nodeID -- cfd
    std::map<int, TcpConnectionPtr> connections_;
    int nodeCount_;
    std::set<int> idleNodeID;//存放当前闲置的nodeID

};

#endif