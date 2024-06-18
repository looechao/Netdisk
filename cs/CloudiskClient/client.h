#include <func.h>
#include <string.h>
#include <unistd.h>
#include <crypt.h>

#define MAXSIZE 4096 // 最大输入长度
#define MAXLINE 1024

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
    CMD_TYPE_PUTS,
    CMD_TYPE_GETS,
    
    CMD_TYPE_USERNAME,
    CMD_TYPE_PASSWD,

    CMD_TYPE_NOTCMD  //不是命令
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

CmdType parse_input(char* str, int peerfd); // 解析命令

int login(int sockfd);
int send_usename(int peerfd);
int send_passwd(int peerfd, train_t* t);

// 一个具体命令的执行
void cdCommand(int sockfd);
void lsCommand(int sockfd);
void pwdCommand(int sockfd);
void mkdirCommand(int sockfd);
void rmdirCommand(int sockfd);
void notCommand(int sockfd);
void putsCommand(int sockfd);
void getsCommand(int sockfd);

