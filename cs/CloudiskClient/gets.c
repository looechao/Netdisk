#include "client.h"

ssize_t Recv_Info(int sockfd,char* buffer,off_t resvsize,int flags);//读操作
ssize_t Send_Info(int sockfd,const char* buffer,int sendsize,int flags);//写操作
int transferFile(int sockfd);

void getsCommand(int sockfd){
    transferFile(sockfd); 
}

int transferFile(int sockfd){
    //默认存储文件夹./default_download
    char default_download[]="./default_download/";

    //接收文件名
    File file={0,{0}};
    Recv_Info(sockfd,(char *)&file.length,sizeof(file.length),0);
    Recv_Info(sockfd,(char *)file.content,file.length,0);

    printf("文件名长度： %d\n",file.length);
    printf("文件名 %s\n",file.content);

    //拼接路径
    char full_path[1024];
    // 初始化 full_path 为 default_download 的内容
    strcpy(full_path,default_download);
    //拼接到 full_path
    strcat(full_path, file.content);
    printf("打开文件： %s\n",full_path);
   
    int fd=open(full_path,O_RDWR | O_CREAT, 0666); 
    ERROR_CHECK(fd,-1,"open failed");
    //接收文件大小
    memset(&file,0,sizeof(file));
    off_t content_size=0;
    Recv_Info(sockfd,(char *)&file.length,sizeof(file.length),0);
    Recv_Info(sockfd,(char *)&file.content,file.length,0);
    memcpy(&content_size,file.content,file.length);
   
    printf("文件大小：%ld\n",content_size);
    if(content_size==0){
        return 0;
    }

    //mmap 接收文件内容
    struct stat file_stat;

    // 使用stat函数获取文件信息
    stat(full_path, &file_stat);
    if(file_stat.st_size!=0){
        file_stat.st_size--;
    }
    // 计算新内容的长度并调整文件大小
    off_t new_size = content_size + file_stat.st_size;
    ftruncate(fd, new_size);

    // 映射新文件内容
    char *p = (char *)mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ERROR_CHECK(p, MAP_FAILED, "mmap");
    
    // 接收文件内容并追加到文件末尾
    
    Recv_Info(sockfd, p + file_stat.st_size, content_size, 0);
    msync(p, new_size, MS_SYNC);
    munmap(p,content_size);

    return 1;    
}

//读操作
ssize_t Recv_Info(int sockfd,char* buffer,off_t resvsize,int flags){
    //清空
       
    off_t size=0;
    while(size<resvsize){
       off_t ret=recv(sockfd,buffer+size,resvsize-size,flags);
       if(ret == -1){
           error(1,errno,"recv failed :sockfd = %d",sockfd);
       }
    
       size+=ret;

    }
    return size;
}

//写操作
ssize_t Send_Info(int sockfd,const char* buffer,int sendsize,int flags){
    
    ssize_t send_size=send(sockfd,buffer,sendsize,flags);
    if(send_size == -1){
        error(1,errno,"send failed");
    }
    
    return send_size;
}
