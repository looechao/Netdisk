#include <func.h>
#include "client.h"

#define MAXSIZE 4096


int main(int argc, char* argv[])
{
    if(argc<3){
        error(1,errno,"argc<3");
    }

    //连接服务器
    int sockfd=My_Connect(argv[1],argv[2]);
    
    //epoll
    int epfd = epoll_create(1);
    ERROR_CHECK(epfd,-1,"epoll_create failed ");

    //输入流，加入监听 
    Epoll_Add(epfd,STDIN_FILENO);
    //当服务端发回消息，加入监听
    
    while(1){
        struct epoll_event events[MAXSIZE];
        int nfp=epoll_wait(epfd,events,MAXSIZE,-1);

        for(int i=0;i<nfp;i++){
            if(events[i].data.fd==STDIN_FILENO){
                printf("输入命令\n");
                send_command(sockfd);
  
            }
              
        }
    }


    return 0;
}
