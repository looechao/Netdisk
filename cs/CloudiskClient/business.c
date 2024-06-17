#include "client.h"

int check_files(train_t *command);

//分析命令
 int send_command(int sockfd){
    train_t command;

    //分析命令，放入结构体
    char buff[1000];
    memset(buff,0,sizeof(buff));
    scanf("%s",buff);

    //判断命令类型
    if(strcmp(buff,"gets")==0){
        printf("buff=%s\n",buff);
        scanf("%s",buff);
        printf("buff=%s\n",buff);
        command.type=CMD_TYPE_GETS;
        command.len=strlen(buff);
        strcpy(command.buff,buff); 
        //检查文件是否存在    
        check_files(&command);
        //发送消息
        printf("已有文件大小：%ld\n",command.file_size);
        Send_Info(sockfd,(char*)&command,sizeof(command.len)+sizeof(command.type)+command.len+sizeof(command.file_size),0); 
        //接受文件
        transferFile(sockfd);
    }
   
    return 1;
}

int check_files(train_t *command){
     //默认存储文件夹./default_download
    char default_download[]="./default_download/";

    char full_path[1024];
    // 初始化 full_path 为 default_download 的内容
    strcpy(full_path, default_download);
    // 将 full_path 拼接到 full_path
    strcat(full_path, command->buff);

    
    struct stat file_stat;

    // 使用stat函数获取文件信息
    if (stat(full_path, &file_stat) == 0) {
        printf("文件 %s 存在。\n", full_path);
        printf("文件大小: %ld 字节\n", file_stat.st_size);
        command->file_size=file_stat.st_size;
    } else {
        printf("文件 %s 不存在或无法访问。\n", full_path);
        command->file_size=0;
    }
    
    return 1;
}


int transferFile(int sockfd){
    //默认存储文件夹./default_download
    char default_download[]="./default_download/";

    //接收文件名
    File file={0,{0}};
    Recv_Info(sockfd,(char *)&file.length,sizeof(file.length),0);
    Recv_Info(sockfd,(char *)file.content,file.length,0);

    printf("%d\n",file.length);
    printf("%s\n",file.content);

    //拼接路径
    char full_path[1024];
    // 初始化 full_path 为 default_download 的内容
    strcpy(full_path, default_download);
    //拼接到 full_path
    strcat(full_path, file.content);
    printf("%s\n",full_path);
   
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

