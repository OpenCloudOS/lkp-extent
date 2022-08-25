#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
using namespace std;
#define NAME "CMDIPC"
#define LEN 4096

int main(int argc, char *argv[])
{
    printf("输入nodeID\n");

    char buf[LEN];

    int sfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        perror("socket fail");
        return 0;
    }

    struct sockaddr_un server, client;

    //设置服务器的地址
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, NAME);

    int ret = connect(sfd, (struct sockaddr *)&server, sizeof(server));
    if (ret < 0)
    {
        perror("listen fail");
        close(sfd);
        unlink(NAME);
        return 0;
    }

    cout << "IPC  connect" << endl;

    char sendline[LEN];
    printf("向CMDserver发送数据\n");
    // for(int i=0;i<LEN;i++){
    //     sendline[i] = i % 26 +  'a';
    // }
    // sendline[LEN - 1] = '\0';
    string input = "0";
    if(argc > 1){
        input = string(argv[1]);
    }
    strcpy(sendline,input.c_str()); 
    send(sfd,sendline,LEN,0);

    
    // while(1){
    //     sleep(1);
    //     recv(sfd,sendline,LEN,0);
    //     cout<<"CMDclient recv:"<<sendline<<endl;
    // }
    
    close(sfd);

    return 0;
}
