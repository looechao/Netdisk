// #include "thread_pool.h"
// #include "log.h"
// #include "databases.h"
// 
// 
// void mkdirCommand(task_t *task, MYSQL *conn) {
//     // 解析命令获取目录路径
//     char *command = task->data;
//     printf("tak->data: %s\n", command);
//     char dir_path[256] = {0};
//     if (sscanf(command, "mkdir %s", dir_path) != 1) {
//         // 命令格式错误
//         sendn(task->peerfd, "Error: Invalid command format", strlen("Error: Invalid command format"));
//         return;
//     }
// 
//     // 拆分路径获取各级目录名
//     char *token = strtok(dir_path, "/");
//     char *dir_list = strdup(dir_path); // 创建目录路径的副本以进行拆分
//     int parent_id = client_users[task->peerfd].pwd_id; // 从当前工作目录开始
// 
//     file_table ptable;
//     memset(&ptable, 0, sizeof(ptable));
//     // 循环创建目录
//     while (token != NULL) {
//         // 设置文件名和父目录 ID
//         strncpy(ptable.file_name, token, sizeof(ptable.file_name) - 1);
//         ptable.file_name[sizeof(ptable.file_name) - 1] = '\0'; // 确保字符串以空字符结尾
//         ptable.parent_id = parent_id;
// 
//         // 检查目录是否存在
//         if (select_file_table(conn, &ptable)) {
//             // 目录已存在
//             char error_msg[256] = {0};
//             sprintf(error_msg, "mkdir: cannot create directory '%s': File exists", token);
//             sendn(task->peerfd, error_msg, strlen(error_msg));
//             free(dir_list);
//             return;
//         }
// 
//         // 目录不存在，添加到数据库
//         if (!add_file_table(conn, &ptable)) {
//             // 添加失败
//             char error_msg[256] = {0};
//             sprintf(error_msg, "mkdir: failed to add '%s' to database", token);
//             sendn(task->peerfd, error_msg, strlen(error_msg));
//             free(dir_list);
//             return;
//         }
// 
//         // 更新 parent_id 为新创建的目录 ID
//         parent_id = ptable.file_id; // 假设 add_file_table 更新了 ptable.file_id
// 
//         // 获取下一个目录名
//         token = strtok(NULL, "/");
//     }
// 
//     // 发送成功消息
//     char success_msg[1024] = {0};
//     sprintf(success_msg, "Directory '%s' created successfully", dir_path);
//     sendn(task->peerfd, success_msg, strlen(success_msg));
// 
//     // 更新用户的工作目录信息
//     client_users[task->peerfd].pwd_id = parent_id;
//     strncpy(client_users[task->peerfd].pwd, dir_path, sizeof(client_users[task->peerfd].pwd) - 1);
//     client_users[task->peerfd].pwd[sizeof(client_users[task->peerfd].pwd) - 1] = '\0'; // 确保字符串以空字符结尾
// 
//     // 释放内存
//     free(dir_list);
// }
