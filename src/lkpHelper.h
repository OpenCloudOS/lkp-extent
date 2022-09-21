#ifndef LKP_HELPER
#define LKP_HELPER

#define COMMENT_CHAR '#'

#include <string>
#include <unordered_map>
#include <map>
#include "muduo/net/Buffer.h"
#include "muduo/net/TcpConnection.h"
#include "lib/lkpProto.pb.h"

// using namespace std;
using namespace muduo;
using namespace muduo::net;

extern std::vector<string> lkpCommands;
extern const std::vector<string> ConfigString;
extern string ROOT_DIR;

typedef struct myLkpConfig{
    uint16_t ServerListenPort;
    uint16_t ServerThreadsNum;
    uint16_t ServerTimeControl;
    uint16_t ServerflushInterval;
    uint16_t ServerPort;
    uint16_t HeartBeatTime;
    string ServerPushPath;
    string ServerResultPath;
    string ServerAddress;
    string ClientPushPath;
} lkpConfig;

bool lkpConfigInit(std::map<string,string> & m, lkpConfig & myConfig, const string & ROOT_DIR);




//将lkpCommand字符串转换为message中使用的enum
bool lkpCmdsToEnum(const string& lkpCmdString, lkpMessage::commandID& lkpEnum);
//将lkpMessage::commandID中的enum转换为字符串
bool lkpEnumToCmds(const lkpMessage::commandID lkpEnum, string& lkpCmdString);

//读取lkp-extend配置文件
bool ReadConfig(const string & filename, std::map<string, string> & m);
//打印配置文件到输出流
void PrintConfig(const std::map<string, string> & m);


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

    //清空信息
    void clear_info();

    //填充信息
    void update_info(int nodeID,string info);

    //获取节点的信息
    string get_info(int nodeID);

private:
    //client pool nodeID -- cfd
    std::map<int, TcpConnectionPtr> connections_;
    
    int nodeCount_;
    std::set<int> idleNodeID;//存放当前闲置的nodeID

    std::map<int, string> node_info;
};

#endif