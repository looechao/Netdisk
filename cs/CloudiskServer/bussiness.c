#include "thread_pool.h"
#include "databases.h"

//主线程调用:处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que)
{
    //消息格式：cmd content
    //1.1 获取消息长度
    int length = -1;
    int ret = recvn(sockfd, &length, sizeof(length));
    printf("recv length: %d\n", length);

    //1.2 获取消息类型
    int cmdType = -1;
    ret = recvn(sockfd, &cmdType, sizeof(cmdType));
    printf("recv cmd type: %d\n", cmdType);

    task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
    ptask->peerfd = sockfd;
    ptask->type=(CmdType)cmdType;

    //还需要传递文件大小，来实现断开重连
    ret = recvn(sockfd,&ptask->file_size,sizeof(ptask->file_size));            
    printf("bussines ptask->file_size=%ld\n",ptask->file_size);

    printf("ptask->data = %d\n", length);
    if(length > 0) {
        //1.3 获取消息内容
        ret = recvn(sockfd, ptask->data, length);
        
        if(ret > 0) {
            //往线程池中添加任务
            taskEnque(que, ptask);
        }
    } else if(length == 0){
        taskEnque(que, ptask);
    }

    if(ret == 0) {//连接断开的情况
        printf("\nconn %d is closed.\n", sockfd);
        delEpollReadfd(epfd, sockfd);
        close(sockfd);
    }
}

//注意：此函数可以根据实际的业务逻辑，进行相应的扩展
void doTask(task_t * task, MYSQL* conn)
{
    assert(task);
    switch(task->type) {
    case CMD_TYPE_PWD:  
        pwdCommand(task);   break;
    case CMD_TYPE_CD:
        cdCommand(task, conn);    break;
    case CMD_TYPE_LS:
        lsCommand(task, conn);    break;
    case CMD_TYPE_MKDIR:
        mkdirCommand(task, conn);  break;
    case CMD_TYPE_RMDIR:
        rmdirCommand(task, conn);  break;
    case CMD_TYPE_RM:
        rmCommand(task, conn);  break;
    // case CMD_TYPE_NOTCMD:
    //     notCommand(task);   break;
    // case CMD_TYPE_PUTS:
    //     putsCommand(task, conn);   break;
    
    case CMD_TYPE_GETS:
        getsCommand(task, conn);   break;
    
    case TASK_LOGIN_USERNAME:
        login_username(task, conn);     break;
    case TASK_LOGIN_PASSWD:
        login_password(task);     break;
    case TASK_REGISTER_USERNAME:
        register_username(task, conn);     break;
    case TASK_REGISTER_PASSWD:
        register_password(task, conn);     break;

    default:
        break;
    }
}
