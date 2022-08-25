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

// #include"help.h"

#define LEN 4096
using namespace std;


int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;
    int n;

    char recvline[LEN], sendline[LEN];

    //创建socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf(" create socket error: %s (errno :%d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(7777);


    //连接
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf(" connect socket error: %s(errno :%d)\n", strerror(errno), errno);
        return 0;
    }

    printf("连接成功\n");

    while (1)
    {
        int ret = recv(sockfd,sendline,LEN,0);
        if(ret < 0){
            perror("wrong!");
        }
        else if(ret == 0){
            cout<<"server end"<<endl;
            close(sockfd);
            break;
        }

        cout<<"TCP client 收到数据:"<<sendline<<endl;

        cout<<"TCP client 尝试向 CMDserver 发送数据"<<endl;
        strcpy(sendline,"this is TCP client results");
        int tmp = send(sockfd,sendline,LEN,0);

        if (tmp <= 0)
        {
            perror("wrong");
        }
    }

    cout<<"TCP client end"<<endl;

    
    return 0;
}
