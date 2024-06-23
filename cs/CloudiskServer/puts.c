#include "thread_pool.h"
#include "databases.h"

int recvn_server(int sockfd, void *buf, int length){
    printf("需要接受：%d\n", length);
    int total = 0;
    char* p = (char *)buf;
    while(total < length){
        ssize_t sret = recv(sockfd, buf+total, length-total, 0);
        if(sret < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                continue;
            }
            perror("recv failed");
            break;
        }
        if(sret == 0){
            printf("Receive failed: connection closed prematurely\n");
            break;
        }
        total += sret;
        printf("已接收: %d 字节\n", total);
    }
    return total;
}



void putsCommand(task_t * task, MYSQL* conn) {
    delEpollReadfd(task->epfd, task->peerfd);
    printf("execute puts command.\n");
    File train;
    // 接收第一个小火车的内容

    recvn_server(task->peerfd, &train.length, sizeof(train.length));
    recvn_server(task->peerfd, &train.content, train.length);
    char* filename = (char *)malloc(128*sizeof(char));
    strcpy(filename,train.content);
    printf("filname: %s\n", filename);
    bzero(&train, sizeof(train));

    // 查询虚拟文件表
    // 初始化虚拟文件表
    file_table f1;
    memset(&f1, 0, sizeof(f1));
    f1.parent_id = client_users[task->peerfd].pwd_id;
    f1.owner_id = client_users[task->peerfd].user_id;
    strcpy(f1.file_name, filename);
    f1.type = 'f';


    // 向虚拟文件表中写一条数据，文件名，用户id
    // 文件名查重

    // 接收第二个小火车的内容
    printf("接收hash\n");
    char* hash_value = (char *)malloc(128*sizeof(char));
    recvn_server(task->peerfd, &train.length, sizeof(train.length));
    recvn_server(task->peerfd, &train.content, train.length);
    memcpy(hash_value, train.content, train.length);  // use memcpy instead
    printf("hash: %s\n", hash_value);
    bzero(&train, sizeof(train));

    // 在虚拟文件表中查找是否存在sha1
    strcpy(f1.sha1, hash_value); 
    int ret = search_file(conn, f1.sha1);
    //文件不存在
    if(ret == -1){
        //发送不秒传消息
        File train = {3, "off"};
        send(task->peerfd, &train, sizeof(train.length)+train.length, MSG_NOSIGNAL);
        bzero(&train, sizeof(train));        
        // // 打开并写入文件
        int fd = open(hash_value, O_RDWR | O_TRUNC | O_CREAT, 0666);
        // if(fd < 0) {
        //     perror("open"); return;
        // }
        // 接收第三个小火车的内容
        off_t filesize;
        printf("接收文件长度\n");    
        recvn_server(task->peerfd, &train.length, sizeof(train.length));
        recvn_server(task->peerfd, train.content, train.length);
        memcpy(&filesize, train.content, sizeof(off_t));
        ftruncate(fd, filesize);
        printf("ftruncate done, filesize = %ld\n", filesize);
        char* p = (char*)mmap(NULL, filesize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        perror("mmap\n");
        printf("接收文件\n");       
        recvn_server(task->peerfd, p, filesize);
        printf("map done\n");
        munmap(p, filesize);
        perror("munmap\n");
        //上传任务执行完毕之后，再加回来
        printf("unmap done\n");
        printf("upload done!\n");
        f1.filesize = filesize;
        add_file_table(conn, &f1);
    }else{//秒传，更新文件表
        File train = {2, "on"};
<<<<<<< HEAD
=======
        // 如果文件名不同|用户不同|父目录不同，则在文件表中添加新的数据
>>>>>>> master
        send(task->peerfd, &train, sizeof(train.length)+train.length, MSG_NOSIGNAL);
        printf("speed_transfer_done!\n");
    }
    addEpollReadfd(task->epfd, task->peerfd);
}
