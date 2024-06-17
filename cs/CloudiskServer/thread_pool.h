#ifndef __WD_FUNC_H
#define __WD_FUNC_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <error.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/sendfile.h>

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

typedef void (*sighandler_t)(int);

#define ARGS_CHECK(argc, num)   {\
    if(argc != num){\
        fprintf(stderr, "ARGS ERROR!\n");\
        return -1;\
    }}

#define ERROR_CHECK(ret, num, msg) {\
    if(ret == num) {\
        perror(msg);\
        return -1;\
    }}

#define THREAD_ERROR_CHECK(ret, func) {\
    if(ret != 0) {\
        fprintf(stderr, "%s:%s\n", func, strerror(ret));\
    }}

typedef enum {
    CMD_TYPE_PWD=1,
    CMD_TYPE_LS,
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
    CMD_TYPE_RMDIR,
    CMD_TYPE_PUTS,
    CMD_TYPE_GETS,
    
    CMD_TYPE_USERNAME,
    CMD_TYPE_PASSWD,
    
    CMD_TYPE_NOTCMD  //不是命令
}CmdType;

//用户结构体
typedef struct{
    struct sockaddr_in clientaddr;// ip
    char directory_address[100];//客户端地址
}User;

typedef struct 
{
    int len;//记录内容长度
    CmdType type;

    //当下载文件时，检查本地是否有相同的文件，并记录大小
    //若文件存在，变量为文件大小，若文件不存在，变量为0
    off_t file_size; 

    char buff[1000];//记录内容本身

}train_t;

 typedef struct{
     int length;                                                                                                             
     char content[4096];
 }File;

typedef struct task_s{
    int peerfd;
    CmdType type;
    off_t file_size;
    char data[1000];
    struct task_s * pNext;
}task_t;

typedef struct task_queue_s
{
    task_t * pFront;
    task_t * pRear;
    int queSize;//记录当前任务的数量
    pthread_mutex_t mutex; 
    pthread_cond_t cond;
    int flag;//0 表示要退出，1 表示不退出

}task_queue_t;

typedef struct threadpool_s {
    pthread_t * pthreads;
    int pthreadNum;
    task_queue_t que;//...任务队列
}threadpool_t;

/*
*日志类型
*@param ACTION_LOGIN:用户登录相关，登录成功或失败
*@param ACTION_INFO:相关命令的执行消息
*@param ACTION_SERVER: 服务器启动关闭相关
*/

typedef enum {
    ACTION_LOGIN    =1,
    ACTION_INFO     =2,
    ACTION_SERVER   =3
} level;

/*
*消息体
*@param log_level 消息等级
*@param msg_length 消息体长度
*@param msg_body 指向消息体首地址的指针
*/

typedef struct msg{
    level log_level;
    char msg_body[128];
}msg;



//全局变量,存储所有用户的地址,索引是用户套接字的值；
extern User client_users[100];

int queueInit(task_queue_t * que);
int queueDestroy(task_queue_t * que);
int queueIsEmpty(task_queue_t * que);
int taskSize(task_queue_t * que);
int taskEnque(task_queue_t * que, task_t * task);
task_t * taskDeque(task_queue_t * que);
int broadcastALL(task_queue_t* que);

int threadpoolInit(threadpool_t *, int num);
int threadpoolDestroy(threadpool_t *);
int threadpoolStart(threadpool_t *);
int threadpoolStop(threadpool_t *);


int tcpInit(const char * ip, const char * port);
int addEpollReadfd(int epfd, int fd);
int delEpollReadfd(int epfd, int fd);
int sendn(int sockfd, const void * buff, int len);
int recvn(int sockfd, void * buff, int len);

//处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que);

//执行task_t * task, msg* message);任务的总的函数
int  doTask(task_t * task, msg* message);
//每一个具体命令的执行
int cdCommand(task_t * task, msg* message);
int lsCommand(task_t * task, msg* message);
int pwdCommand(task_t * task, msg* message);
int mkdirCommand(task_t * task, msg* message);
int rmdirCommand(task_t * task, msg* message);
int notCommand(task_t * task, msg* message);
int putsCommand(task_t * task, msg* message);
int getsCommand(task_t * task, msg* message);
int username_validation(task_t * task, msg* message); 
int password_validation(task_t * task, msg* message);

#endif
