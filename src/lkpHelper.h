#ifndef LKP_HELPER
#define LKP_HELPER

#include <vector>
#include <string>

#include "muduo/net/Buffer.h"
#include "muduo/net/TcpConnection.h"
#include "lib/lkpProto.pb.h"

using namespace std;
extern std::vector<string> lkpCommands;

//将lkpCommand字符串转换为message中使用的enum
bool lkpCmdsToEnum(const string& lkpCmdString, lkpMessage::commandID& lkpEnum);
//将lkpMessage::commandID中的enum转换为字符串
bool lkpEnumToCmds(const lkpMessage::commandID lkpEnum, string& lkpCmdString);

class clientPool : muduo::noncopyable
{
public:
    clientPool();

    
private:


};

#endif