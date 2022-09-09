#include <vector>
#include <string>

#include "lkpHelper.h"

using namespace std;

//commandID和"UPDATE"等字符串的映射关系

std::vector<string> lkpCommands = {
    "UPDATE", "RUN", "RESULT", "PUSH", "LIST"};

bool lkpCmdsToEnum(const string &lkpCmdString, lkpMessage::commandID &lkpEnum)
{
    size_t sz = lkpCommands.size();
    for (int i = 0; i < sz; ++i)
    {
        if (lkpCommands[i].compare(lkpCmdString) == 0)
        {
            lkpEnum = static_cast<lkpMessage::commandID>(i);
            return true;
        }
    }
    return false;
}

bool lkpEnumToCmds(const lkpMessage::commandID lkpEnum, string &lkpCmdString)
{
    if (lkpEnum >= lkpCommands.size() || lkpEnum < 0)
        return false;
    lkpCmdString = lkpCommands[lkpEnum];
    return true;
}


lkpClientPool::lkpClientPool()
{
    connections_.clear();
    nodeCount_ = 0;
    idleNodeID.clear();
}

//客户端总量
int lkpClientPool::size()
{
    return connections_.size();
}

//不是命令行时才可以调用，增加新客户端
int lkpClientPool::add(TcpConnectionPtr conn)
{
    int nodeID = -1;

    //分配新的ID
    if (idleNodeID.empty())
    {
        nodeID = nodeCount_++;
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
    return nodeID;
}

//删除ID
bool lkpClientPool::del(int nodeID)
{
    if (connections_.count(nodeID))
    {
        connections_.erase(nodeID);
        idleNodeID.insert(nodeID);
        return true;
    }
    else
    {
        return false;
    }
}

//获取conn
TcpConnectionPtr lkpClientPool::getConn(int nodeID)
{
    if (connections_.count(nodeID))
    {
        return connections_[nodeID];
    }
    else
    {
        // printf("No such client!\n");
        return NULL;
    }
}

//清空信息
void lkpClientPool::clear_info(){
    node_info.clear();
}

//填充信息
void lkpClientPool::update_info(int nodeID,string info){
    node_info[nodeID] = info;
}

//获取节点的信息
string lkpClientPool::get_info(int nodeID){
    if(node_info.count(nodeID)){
        return node_info[nodeID];
    }
    else{
        return "";
    }
}