#ifndef __WD_CLIENT_H
#define __WD_CLIENT_H

#include <string.h>
#include <unistd.h>
#include <crypt.h>
#include <openssl/evp.h>
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

//哈希表
#include "hashtable.h"


#define MAXSIZE 4096 // 最大输入长度
#define MAXLINE 1024
#define SIZE(a) (sizeof(a)/sizeof(a[0]))

#define THREAD_ERROR_CHECK(ret, func) {\
    if(ret != 0) {\
        fprintf(stderr, "%s:%s\n", func, strerror(ret));\
    }}

#define ERROR_CHECK(ret, num, msg) {\
    if(ret == num) {\
        perror(msg);\
        return -1;\
    }}

typedef enum {
    CMD_TYPE_PWD=1,
    CMD_TYPE_LS,
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
    CMD_TYPE_RMDIR,
    CMD_TYPE_RM,
    CMD_TYPE_PUTS,
    CMD_TYPE_GETS,
    CMD_TYPE_NOTCMD,  //不是命令

    TASK_LOGIN_USERNAME = 100,
    TASK_LOGIN_USERNAME_OK,
    TASK_LOGIN_USERNAME_ERROR,
    TASK_LOGIN_PASSWD,
    TASK_LOGIN_PASSWD_OK,
    TASK_LOGIN_PASSWD_ERROR,
}CmdType;

typedef struct 
{
    int len;//记录内容长度
    CmdType type;
    //若文件存在，变量为文件大小，若文件不存在，变量为0
    off_t file_size;
    char buff[1000];//记录内容本身
} train_t;

typedef struct{
   int length;
   char content[4096];
} File;

//连接
int my_connect(const char* ip,const char* port);
int addEpollReadfd(int epfd, int fd);
int delEpollReadfd(int epfd, int fd);
int recvn(int sockfd, void * buff, int len);
int sendn(int sockfd, const void * buff, int len);

CmdType parse_input(char* str, train_t* information); // 解析命令

int login(int sockfd);

// 一个具体命令的执行
void cdCommand(int sockfd);
void lsCommand(int sockfd);
void pwdCommand(int sockfd);
void mkdirCommand(int sockfd);
void rmdirCommand(int sockfd);
void notCommand(int sockfd);
void putsCommand(int sockfd);
void getsCommand(int sockfd);
void rmCommand(int sockfd);

// 计算文件 SHA-1 
void file_sha1(int fd, char* hash_value);

#endif
