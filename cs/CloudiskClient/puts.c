#include "client.h"

int recvn_client(int sockfd, void *buf, int length){
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

int upload(int fd);

void putsCommand(int sockfd){
    sleep(2);
    upload(sockfd);
}

void send_file(int netfd, int fd, off_t filesize) {
    sleep(2);
    ssize_t sent = sendfile(netfd, fd, NULL, filesize);
    if (sent == -1) {
        perror("发送失败");
    }
    else if (sent == filesize) {
        printf("发送完毕！\n");
    }
    else {
        printf("发送了部分数据，总共发送了 %lld 字节。\n", (long long) sent);
    }
}

int upload(int netfd){
    //共发送三次小火车，中间接收一次内容
    //第一次使用小火车发送文件名
    printf("第一次发送\n");
    File train = {5, "file1"};
    printf("文件名：file1\n");    
    send(netfd, &train, sizeof(train.length)+train.length, MSG_NOSIGNAL);
    bzero(&train, sizeof(train));

    // 打开文件
    int fd = open("file1", O_RDONLY);
    if (fd == -1) {
        perror("open file error");
    }
    // 计算 sha1 值
    char hash_value[41]; 
    file_sha1(fd, hash_value); //假设 file_sha1正确地计算并存储了SHA1值
    fd = open("file1", O_RDONLY);
    printf("%s\n", hash_value);
    // 填充小火车
    train.length = strlen(hash_value);
    memcpy(train.content, hash_value, train.length);
    // 发送小火车
    ssize_t totalLen = sizeof(train.length) + train.length;
    ssize_t ret = send(netfd, &train, totalLen, MSG_NOSIGNAL); // 这里检查返回值以确保所有数据都被发送出去
    if (ret == -1)// 如果上传失败
    {
        perror("send error");
    }
    else if (ret < totalLen)//如果没有上传完毕
    {
        printf("Not all data was sent. Only %zu of %zu bytes were sent\n", ret, totalLen);
    }

    // 清零操作
    bzero(&train, sizeof(train));

    // 接收消息
    recvn_client(netfd, &train.length, sizeof(train.length));
    recvn_client(netfd, &train.content, train.length);
    char* speed_transfer = (char *)malloc(128*sizeof(char));
    strcpy(speed_transfer,train.content);
    printf("speed_transfer: %s\n", speed_transfer);
    bzero(&train, sizeof(train));

    int is_speed_transfer = strcmp(speed_transfer, "off");
    if(is_speed_transfer == 0){
        printf("第三次发送\n");
        // 第三次发送文件大小和内容
        struct stat statbuf;
        fstat(fd, &statbuf);
        printf("filesize = %ld\n", statbuf.st_size);
        train.length = sizeof(statbuf.st_size);
        memcpy(train.content, &statbuf.st_size, train.length);
        send(netfd, &train, sizeof(train.length)+train.length, MSG_NOSIGNAL);
        printf("第四次发送：文件本身\n");
        send_file(netfd, fd, statbuf.st_size);
    }else{// 秒传
        printf("file is uploaded!\n");
    }
    
    return 0;
}

