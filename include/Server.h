#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "NetBuffer.h"
#include "Protocol.h"

/*
int main(int args,const char* argv[]){
    buffer = malloc(buffer_size);

    struct sockaddr_in addr;

    int tcp_socket = socket(AF_INET,SOCK_STREAM,0);
    if(tcp_socket < 0){
        perror("fail to create tcp socket\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi("8003"));

    if(bind(tcp_socket,(struct sockaddr*)&addr,sizeof(addr)) == -1){
        perror("fail to bind tcp socket to port\n");
        return -1;
    }

    if(listen(tcp_socket,5) == -1){
        perror("fail to listen to port\n");
        return -1;
    }

    while(1){
        int fd = accept(tcp_socket,NULL,NULL);
        int words = read(fd,buffer,buffer_size);
        char cmd[512],filename[512];
        sscanf(buffer,"%s%s",cmd,filename);
        if(strcmp(cmd,"GET") == 0){
            handle_get(fd,filename);
        }
        char small_buffer[128];
        int buffer_len = 0;
        while((buffer_len = read(fd,small_buffer,128) != 0)){
            write(1,small_buffer,buffer_len);
        }
        close(fd);   
    }

    close(tcp_socket);

    free(buffer);
    return 0;
}

int print_context(int fd){
    int  buflen = read(fd,buffer,buffer_size);
    write(1,buffer,buflen);
}
*/

struct Player{
    float Position[3];
    float Rotation[3];
};

struct Bullet{
    float Position[3];
    float Speed[3];
    int   id;
};


#define MAX_CLINET_NUM 2
constexpr size_t netBufferSize = 1 << 10;
class Server{
public:
    bool Bind(int port);
    void MainLoop();
    void Close();
private:
    void ParsePost(size_t index);
    void UpdateWorld();

    void ExcuteCommandPlayerPosition(size_t index,std::string& command);
    std::string ParsePlayerPositionCommand(size_t index);

    int tcp_socket;
    fd_set connectedFdSet;
    int connectedFd[MAX_CLINET_NUM];
    int connectedFdNum;
    int maxConnectedFD;

    NetBuffer netBuffer;
    ProtocolPost mPost;

    struct timeval timeout;

    Player players[MAX_CLINET_NUM];
};