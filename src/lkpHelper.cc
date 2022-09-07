#include <vector>
#include <string>

#include "lkpHelper.h"

using namespace std;

std::vector<string> lkpCommands = {
    "UPDATE", "RUN", "RESULT", "PUSH", "LIST"
};

bool lkpCmdsToEnum(const string& lkpCmdString, lkpMessage::commandID& lkpEnum){
    size_t sz = lkpCommands.size();
    for(int i=0; i<sz; ++i){
        if(lkpCommands[i].compare(lkpCmdString)==0){
            lkpEnum = static_cast<lkpMessage::commandID>(i);
            return true;
        } 
    }
    return false;
}

bool lkpEnumToCmds(const lkpMessage::commandID lkpEnum, string& lkpCmdString){
    if(lkpEnum>=lkpCommands.size() || lkpEnum<0)
        return false;
    lkpCmdString = lkpCommands[lkpEnum];
    return true;
}
