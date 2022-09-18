#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "lkpHelper.h"


using namespace std;


const std::vector<string> ConfigString = {
    "ServerListenPort",
    "ServerThreadsNum",
    "ServerTimeControl",
    "ServerflushInterval",
    "ServerPushPath",
    "ServerResultPath",
    "ServerAddress",
    "ServerPort",
    "HeartBeatTime",
    "ClientPushPath"
};

bool lkpConfigInit(std::map<string,string> & m, lkpConfig & myConfig, const string & ROOT_DIR){

    const string CONFIG_PATH = ROOT_DIR + "/lkp-extent.config";
    if(!ReadConfig(CONFIG_PATH, m)){
        std::cout << "lkp-ctl service start failed: Cannot read config file!" << std::endl;
        exit(EXIT_FAILURE);
    }
    for(string config:ConfigString){
        if(!m.count(config)){
            cout << "lkp-ctl service start failed: miss configure: " << config << endl;
            return false;
        }
    }
    myConfig.ServerListenPort    = stoi(m.at(ConfigString[0]));
    myConfig.ServerThreadsNum    = stoi(m.at(ConfigString[1]));
    myConfig.ServerTimeControl   = stoi(m.at(ConfigString[2]));
    myConfig.ServerflushInterval = stoi(m.at(ConfigString[3]));
    myConfig.ServerPushPath      = m.at(ConfigString[4]);
    myConfig.ServerResultPath    = m.at(ConfigString[5]);
    myConfig.ServerAddress       = m.at(ConfigString[6]);
    myConfig.ServerPort          = stoi(m.at(ConfigString[7]));
    myConfig.HeartBeatTime        = stoi(m.at(ConfigString[8]));
    myConfig.ClientPushPath      = m.at(ConfigString[9]);
}

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

        // printf("新客户端加入，nodeID is:%d\n", nodeID);
        // LOG_INFO<<"新客户端加入，nodeID is:"<<nodeID;
    }
    //分配最小的闲置nodeID
    else
    {
        nodeID = *idleNodeID.begin();
        idleNodeID.erase(idleNodeID.begin());
        connections_[nodeID] = conn; //新客户端加入

        // printf("新客户端加入，nodeID is:%d\n", nodeID);
        // LOG_INFO<<"新客户端加入，nodeID is:"<<nodeID;
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


//读取配置文件

//判断是否是空格
bool IsSpace(char c)
{
	if (' ' == c || '\t' == c)
		return true;
	return false;
}

//判断是否是注释
bool IsCommentChar(char c)
{
	if (c == COMMENT_CHAR){
		return true;
	}else{
		return false;
	}
}

void Trim(string & str)
{
	if (str.empty()) {
		return;
	}
	int i, start_pos, end_pos;
	for (i = 0; i < str.size(); ++i) {
		if (!IsSpace(str[i])) {
			break;
		}
	}
	if (i == str.size()) { // 全部是空白字符串
		str = "";
		return;
	}
	start_pos = i;
	for (i = str.size() - 1; i >= 0; --i) {
		if (!IsSpace(str[i])) {
			break;
		}
	}
	end_pos = i;
	str = str.substr(start_pos, end_pos - start_pos + 1);
}

//分析一行语句
bool AnalyseLine(const string & line, string & key, string & value)
{
	if (line.empty())
		return false;
	int start_pos = 0, end_pos = line.size() - 1, pos;
	if ((pos = line.find(COMMENT_CHAR)) != -1) {
		if (0 == pos) {  // 行的第一个字符就是注释字符
			return false;
		}
		end_pos = pos - 1;
	}
	string new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // 预处理，删除注释部分

	if ((pos = new_line.find('=')) == -1)
		return false;  // 没有=号

	key = new_line.substr(0, pos);
	value = new_line.substr(pos + 1, end_pos + 1- (pos + 1));

	Trim(key);
	if (key.empty()) {
		return false;
	}
	Trim(value);
	return true;
}

bool ReadConfig(const string & filename, map<string, string> & m)
{
	m.clear();
	ifstream infile(filename.c_str());
	if (!infile) {
		cout << "lkp-ctl init failed: Config File open error!" << endl;
		return false;
	}
	string line, key, value;
	while (getline(infile, line)) {
		if (AnalyseLine(line, key, value)) {
			m[key] = value;
		}
	}
	infile.close();
	return true;
}

// 打印读取出来的数据
void PrintConfig(const map<string, string> & m)
{
	map<string, string>::const_iterator mite = m.begin();
	for (; mite != m.end(); ++mite) {
		cout << mite->first << "=" << mite->second << endl;
	}
}
