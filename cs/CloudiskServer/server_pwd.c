#include "thread_pool.h"
#include "log.h"
#include "databases.h"
void pwdCommand(task_t * task)

{
    printf("execute pwd command.\n");

    int len=0;
    //获取用户文件描述符
    int sockfd=task->peerfd;

    //获取用户现在的位置（目录位置）
    char msg[4096]={0};
    strcpy(msg,client_users[task->peerfd].pwd);
    len=strlen(msg);
    //将获取的位置发回客户端
    sendn(sockfd, &len, sizeof(int));  
    sendn(sockfd,msg,len);
}
