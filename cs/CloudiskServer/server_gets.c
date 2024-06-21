
#include "thread_pool.h"
#include "log.h"
#include "databases.h"

//读操作
ssize_t Recv_Info(int sockfd,char* buff,off_t readsize,int flags);
//写操作
ssize_t Send_Info(int sockfd,const char* buff,int sendsize,int flags);

int transferFile(int sockfd,char *file_name,off_t file_start);

void splitString(const char * pstrs, char *tokens[], int max_tokens, int * pcount);

extern  log_LockFn my_lock_func;
int getsCommand(task_t * task, MYSQL* conn) {
    printf("execute gets command.\n");
    
    //查找文件是否存在
    //拆分用户输入的路径
    char * strs[20] = {0};
    int cnt = 0;
    //分解字符串
    splitString(task->data, strs, 20, &cnt);
    //循环查询是否存在
    file_table file;
    file.owner_id=client_users[task->peerfd].user_id;
    int parent_id=client_users[task->peerfd].pwd_id;
    char sha1[64];
    memset(sha1,0,sizeof(sha1));
    for(int i=0;i<cnt;i++){
        //赋值文件名,父目录id
        strcpy(file.file_name,strs[i]);
        file.parent_id=parent_id;

        //调用查询
        int ret=select_file_table(conn,&file);
        if(ret==-1){//文件不存在
            //send
            //文件目录不存在 返回给客户端 -1
            File ret_error;
            ret_error.length=-1;
            ret=Send_Info(task->peerfd,(char *)&ret_error,sizeof(ret_error.length),MSG_NOSIGNAL);
            if(ret == -1){//发送失败
                perror("send");
                //message->log_level=ACTION_INFO;
                //strcpy(message->msg_body,"CMD_TYPE_GETS  failled");
                write_log("发送文件名失败","error",my_lock_func);

                return -1;
            }
            return 0;
        }else if(file.type=='f'){

            if(i != cnt-1){// 说明路径中有一个不是目录，是文件
                //文件目录不存在 返回给客户端 -1
                File ret_error;
                ret_error.length=-1;
                ret=Send_Info(task->peerfd,(char *)&ret_error,sizeof(ret_error.length),MSG_NOSIGNAL);
                return -1; 
            }

            strcpy(sha1,file.sha1);
            File send_filename;
            send_filename.length=strlen(file.file_name);
            strcpy(send_filename.content,file.file_name);

            //发送文件名
            ret=Send_Info(task->peerfd,(char *)&send_filename,sizeof(send_filename.length)+send_filename.length,MSG_NOSIGNAL);
            if (ret == -1) {
                perror("send");
                //message->log_level=ACTION_INFO;
                //strcpy(message->msg_body,"CMD_TYPE_GETS  failled");
                write_log("发送文件名失败","error",my_lock_func);
                return -1;
            }
            break;
        }
        parent_id=file.file_id;
    }

    //传输文件内容
    transferFile(task->peerfd,sha1,task->file_size);
    return 0;
}

int transferFile(int sockfd,char *file_name,off_t file_start)
{
    File file;
    //与目录拼接
    // 创建并初始化一个哈希表
    HashTable hash;
    initHashTable(&hash);
    
    //获取配置文件
    int ret=get_conf(&hash);
    if(ret == -1){
        fprintf(stderr,"获取配置文件失败\n");
        return 0;
    }

    char key[]="make_user_address";
    char* foundValue = (char*)find(&hash,key);
    if (foundValue == EMPTY) {
        printf("The key %s does not exist in the hashtable\n", key);
    }

    strcat(foundValue,"/");
    strcat(foundValue,file_name);

    int fd=open(foundValue,O_RDWR,0666);
    if(fd < 0){
       write_log("文件打开失败","error",my_lock_func);
       return -1;
    }
    
    //实现断开，再传输；客户端是否有此文件 ；若有，获取大小；
    //移动文件指针到位置后移n字节
    off_t offset = lseek(fd, file_start, SEEK_SET);
   
    if (offset == -1) {
        fprintf(stderr,"lseek");
         //message->log_level=ACTION_INFO;
        //strcpy(message->msg_body,"CMD_TYPE_GETS  failled");
        write_log("移动文件读指针失败","error",my_lock_func);

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
        fprintf(stderr,"seed");

        //message->log_level=ACTION_INFO;
        //strcpy(message->msg_body,"CMD_TYPE_GETS  failled");
        write_log("发送文件失败","error",my_lock_func);

        close(fd);
        return -1;
    }

    if (file_stat.st_size == 0) { // 不用发文件
        close(fd);
        write_log("发送文件成功","info",my_lock_func);
        return 0;
    }

    // 使用 mmap 发送文件
    char *p =(char *) mmap(NULL, file_stat.st_size+file_start, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr,"mmap");
        //message->log_level=ACTION_INFO;
        //strcpy(message->msg_body,"CMD_TYPE_GETS  failled"); 
        write_log("mmap映射文件失败","error",my_lock_func);

        close(fd);
        return -1;
    }
    if(file_start!=0){
        file_start--;
    }
    Send_Info(sockfd, p+file_start, file_stat.st_size, MSG_NOSIGNAL);
    munmap(p, file_stat.st_size);
    close(fd);
    
    write_log("发送文件成功","info",my_lock_func);

    return 0;
}
//读操作
ssize_t Recv_Info(int sockfd,char* buffer,off_t resvsize,int flags){
    //清空
       
    off_t size=0;
    while(size<resvsize){
       off_t ret=recv(sockfd,buffer+size,resvsize-size,flags);
       if(ret == -1){
           write_log("读取失败","error",my_lock_func);

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
     write_log("发送文件失败","error",my_lock_func);

        return -1;
    }
    
    return send_size;
}

// 假设max_tokens是数组tokens的最大大小
// 在使用后，记得要释放空间
void splitString(const char * pstrs, char *tokens[], int max_tokens, int * pcount) {
    int token_count = 0;
    char *token = strtok((char *)pstrs, "/"); // 使用/分割

    while (token != NULL && token_count < max_tokens - 1) { // 保留一个位置给NULL终止符
        char * pstr = (char*)calloc(1, strlen(token) + 1);
        strcpy(pstr, token);
        tokens[token_count] = pstr;//保存申请的堆空间首地址
        token_count++;
        token = strtok(NULL, "/"); // 继续获取下一个token
    }


    // 添加NULL终止符
    tokens[token_count] = NULL;
    *pcount= token_count;

}

