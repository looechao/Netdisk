#ifndef __WD_THREAD_POOL_H
#define __WD_THREAD_POOL_H
#include "client.h"

typedef struct task_s{
    train_t t;
    struct task_s * pNext;
}task_t;

typedef struct task_queue_s
{
    task_t * pFront;
    task_t * pRear;
    int queSize;//记录当前任务的数量
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int flag;//0 表示要退出，1 表示不退出

}task_queue_t;

typedef struct threadpool_s {
    pthread_t * pthreads;
    int pthreadNum;
    task_queue_t que;//...任务队列
}threadpool_t;


int queueInit(task_queue_t * que);
int queueDestroy(task_queue_t * que);
int queueIsEmpty(task_queue_t * que);
int taskSize(task_queue_t * que);
int taskEnque(task_queue_t * que, task_t * task);
task_t * taskDeque(task_queue_t * que);
int broadcastALL(task_queue_t* que);

int threadpoolInit(threadpool_t *, int num);
int threadpoolDestroy(threadpool_t *);
int threadpoolStart(threadpool_t *);
int threadpoolStop(threadpool_t *);

int addEpollReadfd(int epfd, int fd);
int delEpollReadfd(int epfd, int fd);

//执行task_t * task, msg* message);任务的总的函数
void doTask(task_t * task);

#endif
