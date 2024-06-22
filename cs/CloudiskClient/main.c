#include "client.h"
#include "thread_pool.h"

#define EPOLL_ARR_SIZE 100

int main(void) {
    //连接服务器
     int sockfd = my_connect("127.0.0.1", "8080");
                                                                  
    //函数在login.c
    char username[10] = { 0 };
    login(sockfd, username);

    //创建线程池结构体
    threadpool_t threadpool;
    memset(&threadpool, 0, sizeof(threadpool));
    // 初始化线程池
    threadpoolInit(&threadpool, 1);
    // 启动线程池
    threadpoolStart(&threadpool);

    // 创建epoll实例
    //int epfd = epoll_create1(0);
    //ERROR_CHECK(epfd, -1, "epoll_create1");
    //struct epoll_event* pEventArr = (struct epoll_event*)calloc(EPOLL_ARR_SIZE, sizeof(struct epoll_event));

    char str[MAXLINE];

    for (; ; ) {
        memset(str, 0, sizeof(str));
        printf("%s$>> ", username); // 提示符
        fflush(stdout);
        // char * ret=fgets(str, MAXLINE, stdin); // 从stdin读取命令
        // if(ret==NULL){
        //     printf("\n");
        //     break;    
        // }
        
        ssize_t ret = read(STDIN_FILENO, str, MAXLINE);
        if (ret == 0) {
            putchar('\n');
            break;
        }
        
        if (strcmp(str, "exit\n") == 0) { // 检查是否为exit命令
            break; // 退出循环
        }
        
        // 解析命令  返回train_t
        train_t t;
        CmdType type = parse_input(str, &t);
        
        // 长命令加入任务队列；短命令直接处理 
        if (type == CMD_TYPE_GETS || type == CMD_TYPE_PUTS) {
            task_t* ptask = (task_t*)calloc(1, sizeof(task_t));
            ptask->t = t;
            taskEnque(&threadpool.que, ptask);
        }
        else {
            sendn(sockfd, &t, sizeof(t.len)+sizeof(t.type)+sizeof(t.file_size)+ t.len);
        }

        // 接收服务器返回的信息 #所有人自己创建.c文件，如：gets.c文件存放 getsCommand(sockfd);   
        switch (type) {
        case CMD_TYPE_PWD:  
            pwdCommand(sockfd);   
            break;
        case CMD_TYPE_CD:
            cdCommand(sockfd, username);    
            break;
        case CMD_TYPE_LS:
            lsCommand(sockfd);    
            break;
        case CMD_TYPE_MKDIR:
            mkdirCommand(sockfd);  
            break;
        case CMD_TYPE_RMDIR:
            rmdirCommand(sockfd);  
            break;
        case CMD_TYPE_RM:
            rmCommand(sockfd);  
            break;
        case CMD_TYPE_NOTCMD:
            notCommand();   
       /* case CMD_TYPE_NOTCMD:
            notCommand(sockfd);   
>>>>>>> master
            break;
       * case CMD_TYPE_PUTS:
            putsCommand(sockfd);   
            break;
        case CMD_TYPE_GETS:
            getsCommand(sockfd);   
<<<<<<< HEAD
            break;
=======
            break;*/
        default:
            break;
        }
    }
    close(sockfd);

    return 0;
}

