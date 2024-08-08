# 交互时序

``` bash
     客户端                 服务端                
        |                    |
     puts命令---------------->|            
        |         接收命令消息,关闭EPOLL监听
        |                    |
        |<------文件名查重，发送是否传送消息
     接收消息                 |
        |                    |
 发送sha1（或停止)------------>|
        |               接收sha1-----------------或停止
        |                    |                     |
        |<-----------sha1查重，发送是否传秒传--秒传    |
        |                    |               |     |
  sendfile(或秒传)----------->|               |     |
                          接收文件            |     |
                             |               |     |
                           更新表-------------|     |
                             |                     |
                        打开EPOLL监听---------------|
```

## 上传功能的实现

1. 服务端

   ```c
   void putsCommand(task_t * task, MYSQL* conn) {
       delEpollReadfd(task->epfd, task->peerfd);   
       printf("execute puts command.\n");
       File train;
       memset(&train, 0, sizeof(train));
       // 初始化虚拟文件表
       file_table f1;
       memset(&f1, 0, sizeof(f1));
       f1.parent_id = client_users[task->peerfd].pwd_id;
       f1.owner_id = client_users[task->peerfd].user_id;
       strcpy(f1.file_name, task->data);
       f1.type = 'f';
   
       //文件名查重后，向客户端发送是否可传的消息
   	int ret = select_file_table(conn, &f1);
       if (ret == 0) {
           send(task->peerfd, "off", 3, 0);
           addEpollReadfd(task->epfd, task->peerfd);
           return;
       }
       send(task->peerfd, "on", 2, 0);
   	
       
       // 接收客户端发来的hash数值（此时是sha-1）
       printf("接收hash\n");
       char hash_value[64] = { 0 };
       recvn_server(task->peerfd, &train.length, sizeof(train.length));
       recvn_server(task->peerfd, &train.content, train.length);
       memcpy(hash_value, train.content, train.length); 
       printf("hash: %s\n", hash_value);
       bzero(&train, sizeof(train));
   
       // 在虚拟文件表中查找是否存在sha1
       strcpy(f1.sha1, hash_value); 
       ret = search_file(conn, hash_value);
   
       // 接收第三个小火车的内容
       off_t filesize;
       printf("接收文件长度\n");    
       recvn_server(task->peerfd, &train.length, sizeof(train.length));
       recvn_server(task->peerfd, train.content, train.length);
       memcpy(&filesize, train.content, sizeof(off_t));
       f1.file_size = filesize;
       printf("filesize = %ld\n", filesize);
   
       //文件不存在
       if(ret == -1){
           //发送不秒传消息
           File train = {3, "off"};
           send(task->peerfd, &train, sizeof(train.length)+train.length, MSG_NOSIGNAL);
           bzero(&train, sizeof(train));        
           // // 打开并写入文件
           char temp[4096] = {0};
           sprintf(temp, "./User/%s", hash_value);
           printf("temp = %s\n", temp);
           int fd = open(temp, O_RDWR | O_TRUNC | O_CREAT, 0666);
           // 接收sendfile 发来的逻辑
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
   		// 更新虚拟文件表
           add_file_table(conn, &f1);
       }else{//秒传，并直接更新文件表
           File train = {2, "on"};
           send(task->peerfd, &train, sizeof(train.length)+train.length, MSG_NOSIGNAL);
           add_file_table(conn, &f1);
           printf("speed_transfer_done!\n");
       }
       addEpollReadfd(task->epfd, task->peerfd);
   }
   ```

   难点：

   - 接收文件要关掉EPOLL监听

2. 客户端

   ```c
   void putsCommand(int sockfd, const char* filename){
       sleep(2);
       char msg[4] = {0};
       // 如果文件名存在，不进行上传
       ssize_t ret = recv(sockfd, msg, sizeof(msg), 0);
       if (ret == 2) {
           upload(sockfd, filename);
       }
       else {
           puts("文件名已存在");
       }
   }
   ```

   

   ```c
   int upload(int netfd, const char* filename){
       File train;
       bzero(&train, sizeof(train));
       // 打开文件
       char temp[4096] = {0}; 
       sprintf(temp, "./User/%s", filename);
       int fd = open(temp, O_RDONLY);
       if (fd == -1) {
           perror("open file error");
       }
       // 计算 sha1 值
       char hash_value[41]; 
       file_sha1(fd, hash_value); //假设 file_sha1正确地计算并存储了SHA1值
       fd = open(temp, O_RDONLY);
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
       // 第三次发送文件大小和内容
       struct stat statbuf;
       fstat(fd, &statbuf);
       printf("filesize = %ld\n", statbuf.st_size);
       train.length = sizeof(statbuf.st_size);
       memcpy(train.content, &statbuf.st_size, train.length);
       send(netfd, &train, sizeof(train.length)+train.length, MSG_NOSIGNAL);
   
       bzero(&train, sizeof(train));
       // 接收是否妙传的消息
       recvn_client(netfd, &train.length, sizeof(train.length));
       recvn_client(netfd, &train.content, train.length);
       char* speed_transfer = (char *)malloc(128*sizeof(char));
       strcpy(speed_transfer,train.content);
       printf("speed_transfer: %s\n", speed_transfer);
       bzero(&train, sizeof(train));
       
       int is_speed_transfer = strcmp(speed_transfer, "off");
       if(is_speed_transfer == 0){
           printf("发送文件\n");
           send_file(netfd, fd, statbuf.st_size);
       }else{// 秒传
           printf("file is uploaded!\n");
       }
       
       return 0;
   }
   
   ```

## 相关SQL语句

- 文件查重

  ```c
  const char* sql = "SELECT * FROM user_table WHERE user_name = ?";
  ```

- 哈希查重（sha1）

  ```c
  const char* cmd = "SELECT file_name, file_size, type FROM file_table WHERE parent_id = ? AND owner_id = ? ";
  ```

  
