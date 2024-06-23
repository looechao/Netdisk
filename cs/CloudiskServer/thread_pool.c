#include "thread_pool.h"
#include <bits/pthreadtypes.h>
#include "databases.h"
#include "linked_list.h"
#include "hashtable.h"

//声明全局变量
user_table client_users[100];

extern ListNode* arr_queue[30];
extern HashTable last_active;
extern int cs;
extern MYSQL* conn;
//---------------------------------------------------------------------------------1
//每一个子线程在执行的函数执行体(start_routine)
void * threadFunc(void* arg)
{
    //不断地从任务队列中获取任务，并执行
    threadpool_t * pThreadpool = (threadpool_t*)arg;

    while(1) {
        task_t * ptask = taskDeque(&pThreadpool->que);
        if(ptask) {
            //执行业务逻辑
            doTask(ptask, conn);
            free(ptask);//执行完任务后，释放任务节点
        } else {//ptask为NULL的情况
            break;
        }
    }
    printf("sub thread %ld is exiting.\n", pthread_self());
    return NULL;
}

//每一个子线程在执行的函数执行体(start_routine)
void * thread_time_Func(void* arg)
{
    while(1) {
        // 超时的需要踢出
        time_t curtime_index = time(NULL) % 30;
        ListNode* del_head = arr_queue[curtime_index];

        //int jointime_index = (curtime_index + 30 - 1) % 30;     
        //ListNode* add_head = arr_queue[jointime_index];         
        //char key[50];                                           
        //int fd = 10;
        //sprintf(key, "%d", fd);                                  
        //appendNode(&add_head, (void*)key);                      

        
        while (del_head->next != NULL) {
            
            printf("踢了一个");
            ListNode* curr = del_head-> next;
            int del_fd = *((int*)curr->val);
            // 关闭连接
            close(del_fd);
            del_head->next = curr->next;
            free(curr);
    
            char key[50];
            sprintf(key, "%d", del_fd);
            erase(&last_active, key);
            printf("Connection %d has been closed due to inactivity.\n", del_fd);
        }
        sleep(1);

    }
    printf("sub thread %ld is exiting.\n", pthread_self());
    return arg;
}

//---------------------------------------------------------------------------------1

int threadpoolInit(threadpool_t * pthreadpool, int num)
{
    pthreadpool->pthreadNum = num;
    pthreadpool->pthreads =(pthread_t *) calloc(num, sizeof(pthread_t));
    queueInit(&pthreadpool->que);

    return 0;
}

int threadpoolDestroy(threadpool_t * pthreadpool)
{
    free(pthreadpool->pthreads);
    queueDestroy(&pthreadpool->que);
    return 0;
}
//-------------------------------------------------------------------------------------2
int threadpoolStart(threadpool_t * pthreadpool)
{
    if(pthreadpool) {
        for(int i = 0; i < pthreadpool->pthreadNum; ++i) {
            int ret = pthread_create(&pthreadpool->pthreads[i],
                           NULL,
                           threadFunc, pthreadpool);
            THREAD_ERROR_CHECK(ret, "pthread_create");
        }
    }
    return 0;
}
int threadpool_time_Start(threadpool_t * pthreadpool)
{
    if(pthreadpool) {
        for(int i = 0; i < pthreadpool->pthreadNum; ++i) {
            int ret = pthread_create(&pthreadpool->pthreads[i],
                           NULL,
                           thread_time_Func,NULL);
            THREAD_ERROR_CHECK(ret, "pthread_create");
        }
    }
    return 0;
}


//-------------------------------------------------------------------------------------2
int threadpoolStop(threadpool_t * pthreadpool)
{
    //如果任务队列中还有任务，先等待一下，所有任务执行完毕之后
    //再发广播，退出
    while(!queueIsEmpty(&pthreadpool->que))
    {
        sleep(1);//每一个线程都可以sleep
    }
    
    //发广播, 通知所有的子线程退出
    broadcastALL(&pthreadpool->que);
    //回收所有子线程的资源
    for(int i = 0; i < pthreadpool->pthreadNum; ++i) {
        pthread_join(pthreadpool->pthreads[i], NULL);
    }
    return 0;
}

