// #include "client.h"
// 
// void lsCommand(int sockfd) {
//     int len;
//     char *file_list = NULL;
// 
//     // 接收文件列表的长度
//     if (recv(sockfd, &len, sizeof(int), 0) != sizeof(int)) {
//         perror("Error receiving file list length");
//         return;
//     }
// 
//     // 分配内存
//     file_list = (char *)malloc(len + 1); // 分配len+1字节，包括结尾的'\0'
//     if (!file_list) {
//         perror("Memory allocation failed");
//         return;
//     }
// 
//     // 接收实际的文件列表
//     ssize_t bytes_read = 0;
//     while (bytes_read < len) {
//         ssize_t result = recv(sockfd, file_list + bytes_read, len - bytes_read, 0);
//         if (result <= 0) {
//             if (result == 0) {
//                 fprintf(stderr, "Server closed the connection unexpectedly\n");
//             } else {
//                 perror("Error receiving file list");
//                 exit(EXIT_FAILURE);
//             }
//             free(file_list);
//             return;
//         }
//         bytes_read += result;
//     }
// 
//     // 添加字符串结尾的'\0'
//     file_list[len] = '\0';
// 
//     // 打印接收到的文件列表
//     printf("File list:\n%s", file_list);
// 
//     printf("\n");
//     // 释放内存
//     free(file_list);
// }
// 
