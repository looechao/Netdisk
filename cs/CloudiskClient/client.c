#include "client.h"

//创造连接
int My_Connect(const char* ip,const char* port){
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1){
        error(1,errno,"sockfd creat failed");
    }
    
    struct sockaddr_in addr;
    //ip转换
    int ret=inet_aton(ip, &addr.sin_addr);
    if(ret == 0){
        error(1,1,"server.c: inet_aton failed\n");
    } 
    //端口转换
    addr.sin_port=htons(atoi(port));
    //设置ipv4
    addr.sin_family=AF_INET;

    ret=connect(sockfd,(struct sockaddr *)&addr,sizeof(addr));
    if(ret==-1){
        error(1,errno,"server.c : bind failed\n");
    }
    
    return sockfd;
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


