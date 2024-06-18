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

