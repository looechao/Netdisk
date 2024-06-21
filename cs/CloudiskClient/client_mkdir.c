#include "client.h"

void mkdirCommand(int sockfd) {
    char buffer[1024]; 
    ssize_t bytesRead;

    // 接收服务器的响应，直到遇到换行符或缓冲区满
    bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0); // 留个'\0'
    if (bytesRead <= 0) {
        perror("Error receiving data from server");
        return;
    }

    // 确保字符串以null终止（如果recv没有遇到换行符就返回了）
    buffer[bytesRead] = '\0';

    // 查找并移除可能的换行符
    char *newline = strchr(buffer, '\n');
    if (newline != NULL) {
        *newline = '\0'; // 替换换行符为字符串结束符
    }

    // 打印
    puts(buffer);
}

