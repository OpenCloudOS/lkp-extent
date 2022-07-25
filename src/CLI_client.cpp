#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

#define BACK_LOG 100
#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

/*
* TO DO: 添加信号退出控制，超时报警
*/

void handleError(char* msg) { 
    perror(msg);
    exit(-1);
}

int main(int argc, char* argv[]){
    if(argc != 2)
        handleError("Usage: cli-client <sockpath> <command>");

    const char* path = argv[1];
    const char* command = argv[2];
    unlink(path);

    int cli_socket = socket(PF_UNIX, SOCK_STREAM, 0);
    if(cli_socket == -1)
        handleError("client socket error");
    
    struct sockaddr_un addr;
    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    if (connect(cli_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        handleError("connect error");
    }
        
    write(cli_socket, command, strlen(command));

    while(true){
        if(read(cli_socket, buffer, BUFFER_SIZE) == 0)
            return 0;
        fputs(buffer, stdout);
    }

    return 0;
    
}





