#include "thread_pool.h"
//读操作
ssize_t Recv_Info(int sockfd,char* buff,off_t readsize,int flags);
//写操作
ssize_t Send_Info(int sockfd,const char* buff,int sendsize,int flags);

int transferFile(int sockfd,char *file_address,off_t file_start);


void putsCommand(task_t * task) {
    printf("execute puts command.\n");
}

void getsCommand(task_t * task) {
    printf("execute gets command.\n");
    //获取文件位置
    char file_address[4096];
    strcpy(file_address,client_users[task->peerfd].directory_address);
    
    if(task->data[0]!='/'){
        strcat(file_address,"/");
    }

    strcat(file_address,task->data);
    puts(file_address);

    transferFile(task->peerfd,file_address,task->file_size);

}

int transferFile(int sockfd,char *file_address,off_t file_start)
{
    printf("开始传文件\n");
    //发送文件名
    char *filename;

    // 使用 strrchr 函数找到最后一个 '/' 字符的位置
    filename = strrchr(file_address, '/');
    filename++;
    File file={0,{0}};
    file.length=strlen(filename);
    strcpy(file.content, filename);
    puts(filename);
    int ret=Send_Info(sockfd,(char *)&file,sizeof(file.length)+file.length,MSG_NOSIGNAL);
    if (ret == -1) {
        perror("send");
        //message->log_level=ACTION_INFO;
        //strcpy(message->msg_body,"CMD_TYPE_GETS  failled");
        return -1;
    }

    int fd=open(file_address,O_RDWR,0666);
    //实现断开，再传输；客户端是否有此文件 ；若有，获取大小；
    // 移动文件指针到位置后移n字节
    off_t offset = lseek(fd, file_start, SEEK_SET);
    if (offset == -1) {
        perror("lseek");
         //message->log_level=ACTION_INFO;
        //strcpy(message->msg_body,"CMD_TYPE_GETS  failled");
        close(fd);
        return -1;
    }

    //发送文件大小 //假设file_start永远小于 file_stat.st_size; 
    struct stat file_stat;
    fstat(fd,&file_stat);
    file_stat.st_size -=file_start;
    printf("已有文件长度%ld,发送文件长度：%ld\n",file_start,file_stat.st_size);

    file.length=sizeof(file_stat.st_size);
    memcpy(file.content,&file_stat.st_size,file.length);
    ret=Send_Info(sockfd,(char *)&file,sizeof(file.length)+file.length,MSG_NOSIGNAL);
    if (ret == -1) {
        perror("send");
         //message->log_level=ACTION_INFO;
        //strcpy(message->msg_body,"CMD_TYPE_GETS  failled");
        
        close(fd);
        return -1;
    }


    if (file_stat.st_size == 0) { // 不用发文件
        close(fd);
        return 0;
    }

    // 使用 mmap 发送文件
    char *p =(char *) mmap(NULL, file_stat.st_size+file_start, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        //message->log_level=ACTION_INFO;
        //strcpy(message->msg_body,"CMD_TYPE_GETS  failled"); 
        close(fd);
        return -1;
    }
    if(file_start!=0){
        file_start--;
    }
    Send_Info(sockfd, p+file_start, file_stat.st_size, MSG_NOSIGNAL);
    munmap(p, file_stat.st_size);
    close(fd);
    printf("发送结束\n");

    return 0;
}
//读操作
ssize_t Recv_Info(int sockfd,char* buffer,off_t resvsize,int flags){
    //清空
       
    off_t size=0;
    while(size<resvsize){
       off_t ret=recv(sockfd,buffer+size,resvsize-size,flags);
       if(ret == -1){
           return -1;
       }
    
       size+=ret;

    }
    return size;
}

//写操作
ssize_t Send_Info(int sockfd,const char* buffer,int sendsize,int flags){
    
    ssize_t send_size=send(sockfd,buffer,sendsize,flags);
    if(send_size == -1){
        return -1;
    }
    
    return send_size;
}


