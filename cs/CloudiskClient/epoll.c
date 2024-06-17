#include "client.h"

//添加监听
int Epoll_Add(int epfd,int sockfd){
    struct epoll_event event;
    event.events=EPOLLIN;
    event.data.fd=sockfd;
    int ret=epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event);
    if(ret == -1){
        error(1,errno,"epoll.c :epoll_add  epoll_ctl failed");
    }

    return 0;
}

//关闭监听
int Epoll_Delete(int epfd,int sockfd){
    int ret = epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL);
    if(ret==-1){
        error(1,errno,"epoll.c:epoll_delete epoll_ctl failed");
    }
    return 0;
}
