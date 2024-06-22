#include "client.h"

void pwdCommand(int sockfd) {
    int len;
    // 接收数据的长度
    if (recv(sockfd, &len, sizeof(int), 0) <= 0) {
        perror("Error receiving length");
        return;
    }

    // 根据接收到的长度来分配内存
    char *msg = (char *)malloc(len + 1); // 分配len+1字节，包括结尾的'\0'
    if (msg == NULL) {
        perror("Memory allocation failed");
        return;
    }

    // 接收实际的数据
    if (recv(sockfd, msg, len, 0) != len) {
        perror("Error receiving data");
        free(msg); //释放内存
        return;
    }

    // 添加字符串结尾的'\0'
    msg[len] = '\0';

    // 打印
    puts(msg);

    // 释放内存
    free(msg);
}

