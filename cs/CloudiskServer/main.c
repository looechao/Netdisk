#include "thread_pool.h"
#include "log.h"
#define EPOLL_ARR_SIZE 100

int exitPipe[2];
//创建全局锁
pthread_mutex_t lock;
pthread_mutexattr_t attr;

#include "linked_list.h"
#include "user.h"

#define EPOLL_ARR_SIZE 100

ListNode * userList = NULL;
void sigHandler(int num)
{
    printf("\n sig is coming.%d\n",num);
    //激活管道, 往管道中写一个1
    int one = 1;
    write(exitPipe[1], &one, sizeof(one));
}

log_LockFn my_lock_func = my_lock_function;

int main(void)
{   
    // 创建并初始化一个哈希表
    HashTable hash;
    initHashTable(&hash);
    
    //获取配置文件
    int ret=get_conf(&hash);
    if(ret == -1){
        fprintf(stderr,"获取配置文件失败\n");
        return 0;
    }


    //ip,port,threadNum
    printf("sizeof(CmdType):%lu\n", sizeof(CmdType));
    //创建匿名管道
    pipe(exitPipe);

    //fork之后，将创建了子进程
    pid_t pid = fork();
    if(pid > 0) {//父进程
        close(exitPipe[0]);//父进程关闭读端
        signal(SIGUSR1, sigHandler);
        
        wait(NULL);//等待子进程退出，回收其资源
        close(exitPipe[1]);
        printf("\nparent process exit.\n");
        
        //销毁哈希表
        destroyHashTable(&hash);

        exit(0);//父进程退出
    }
    //子进程
    close(exitPipe[1]);//子进程关闭写端
    //打开日志文件流
    FILE *fp = fopen("./sys.log","a");
    if(fp){ 
        log_add_fp(fp,LOG_INFO);
    }else{
        exit(1);

    }

    //true - 关闭控制台写日志；
    log_set_quiet(true);
    //设置锁为递归锁
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lock, &attr);

    //获取锁
    log_set_lock(my_lock_func,NULL);
    my_lock_func(true,NULL);
    //写入日志信息
    log_info("server started !");
    //释放锁
    my_lock_func(false,NULL);

    threadpool_t threadpool;
    memset(&threadpool, 0, sizeof(threadpool));

    //初始化线程池
    char key[100]="thread_num";
    char* foundValue = (char*)find(&hash,key);
    if (foundValue == EMPTY) {
        printf("The key %s does not exist in the hashtable\n", key);
    }
    threadpoolInit(&threadpool,atoi(foundValue));
    //启动线程池
    threadpoolStart(&threadpool);

    strcpy(key,"ip");
    foundValue = (char*)find(&hash,key);
    if (foundValue == EMPTY) {
        printf("The key %s does not exist in the hashtable\n", key);
    }
    strcpy(key,"port");
    char* foundValue2 = (char*)find(&hash,key);
    if (foundValue == EMPTY) {
        printf("The key %s does not exist in the hashtable\n", key);
    }
    
    //创建监听套接字
    int listenfd = tcpInit(foundValue, foundValue2);

    //创建epoll实例
    int epfd = epoll_create1(0);
    ERROR_CHECK(epfd, -1, "epoll_create1");

    //对listenfd进行监听
    addEpollReadfd(epfd, listenfd);
    addEpollReadfd(epfd, exitPipe[0]);

    struct epoll_event * pEventArr = (struct epoll_event*)
        calloc(EPOLL_ARR_SIZE, sizeof(struct epoll_event));
    while(1) {
        int nready = epoll_wait(epfd, pEventArr, EPOLL_ARR_SIZE, -1);
        if(nready == -1 && errno == EINTR) {
            continue;
        } else if(nready == -1) {
            ERROR_CHECK(nready, -1, "epoll_wait");
        } else {
            //大于0
            for(int i = 0; i < nready; ++i) {
                int fd = pEventArr[i].data.fd;
                if(fd == listenfd) {//对新连接进行处理
                    struct sockaddr_in clientaddr;
                    // 接受连接
                    socklen_t client_addr_size = sizeof(clientaddr);
                    int peerfd = accept(listenfd, (struct sockaddr*)&clientaddr, &client_addr_size);
                    printf("\n conn %d has conneted.\n", peerfd);
                    //将新连接添加到epoll的监听红黑树上
                    addEpollReadfd(epfd, peerfd);
                    //将默认工作目录,用户ip放入用户结构体,
                    
                    strcpy(key,"make_user_address");
                    foundValue = (char*)find(&hash,key);
                    if (foundValue == EMPTY) {
                        printf("The key %s does not exist in the hashtable\n", key);
                    }


                    strcpy(client_users[peerfd].directory_address,"./User");    
                    client_users[peerfd].clientaddr=clientaddr;
                     //添加用户节点
                    user_t * user = (user_t*)calloc(1, sizeof(user_t));
                    user->sockfd = peerfd;
					appendNode(&userList, user);
                } else if(fd == exitPipe[0]) {
                    //线程池要退出
                    int howmany = 0;
                    //对管道进行处理
                    read(exitPipe[0], &howmany, sizeof(howmany));
                    //主线程通知所有的子线程退出
                    threadpoolStop(&threadpool);
                    //子进程退出前，回收资源
                    threadpoolDestroy(&threadpool);
                    //销毁哈希表
                    destroyHashTable(&hash);
                    close(listenfd);
                    close(epfd);
                    close(exitPipe[0]);
                    //关闭目录流
                    fclose(fp);
                    pthread_mutex_destroy(&lock);
                    pthread_mutexattr_destroy(&attr);
                    printf("\nchild process exit.\n");
                    exit(0);
                } else {//客户端的连接的处理
                    handleMessage(fd, epfd, &threadpool.que);
                }
            }
        }
    }
    return 0;
}

