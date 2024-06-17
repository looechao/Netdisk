
/*
 *@author lwh created
 *
 *
 *
 */
#include "thread_pool.h"

void pwdCommand(task_t * task)
{
    //获取用户文件描述符
    int sockfd=task->peerfd;

    //获取用户现在的位置（目录位置）
    char addr[1024]={0};
    strcpy(addr,client_users[sockfd].directory_address);
    
    //将获取的位置发回客户端
    sendn(sockfd,addr,sizeof(addr));
}
