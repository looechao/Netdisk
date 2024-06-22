#include "client.h"

//创造连接
int my_connect(const char* ip, const char* port){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
     ERROR_CHECK(sockfd, -1, "sockfd");

    struct sockaddr_in addr;
    //ip转换
    int ret = inet_aton(ip, &addr.sin_addr);
    ERROR_CHECK(ret, -1, "inet_aton");

    //端口转换
    addr.sin_port = htons(atoi(port));
    //设置ipv4
    addr.sin_family=AF_INET;

    ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    ERROR_CHECK(ret, -1, "connect");

    return sockfd;
}

int addEpollReadfd(int epfd, int fd) {
    struct epoll_event evt;
    memset(&evt, 0, sizeof(evt));
    evt.data.fd = fd;
    evt.events = EPOLLIN;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);
    ERROR_CHECK(ret, -1, "epoll_ctl");
    return 0;
}

int delEpollReadfd(int epfd, int fd) {
    struct epoll_event evt;
    memset(&evt, 0, sizeof(evt));
    evt.data.fd = fd;
    evt.events = EPOLLIN;
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &evt);
    ERROR_CHECK(ret, -1, "epoll_ctl");
    return 0;
}
