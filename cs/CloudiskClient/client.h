#include <func.h>
#define MAXLINE 1024 // 最大输入长度

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
int My_Connect(const char* ip,const char* port);

ssize_t Recv_Info(int sockfd,void* buff,off_t readsize,int flags);//读操作
ssize_t Send_Info(int sockfd,const void* buff,int sendsize,int flags);//写操作

int Epoll_Add(int epfd,int sockfd);//添加监听
int Epoll_Delete(int epfd,int sockfd);//删除监听

int transferFile(int sockfd);//上传文件

int send_command(int sockfd);//分析命令
                        
int prase(); // 解析命令
