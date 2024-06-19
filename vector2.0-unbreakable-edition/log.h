#ifndef __LOG_H__
#define __LOG_H__

#define MAX_MSG_SIZE 1024
#include <stdio.h>
#include <mqueue.h>
#include <stdlib.h>
#include "thread_pool.h"

/*
 * 初始化日志消息队列
 * @param queue_name 日志消息队列名字
 * @param attr 指向日志消息体的指针
 *
*/
mqd_t init_message_queue(const char *queue_name, struct mq_attr *attr);


/*
 * 向日志消息队列写入消息
 * @param mqdes 日志消息队列的文件描述符
 * @param message 指向消息结构体的指针
 *
*/
int write_to_queue(mqd_t mqdes, msg* messsge);

//从日志消息队列中取出消息
int read_from_queue_and_log(mqd_t mqdes, const char *log_file_path);

#endif 
