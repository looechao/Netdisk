#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "log.h"
#include "thread_pool.h"

//获取当前时间
const char* get_time(void){
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char* time_str = (char*)malloc(100* sizeof(char));
    if(time_str == NULL){ 
        error(1,errno,"malloc in get_time\n");
    }
    strftime(time_str, 100,
                "%Y-%m-%d %H:%M:%S", local_time);
    return time_str;
} 

//将日志等级枚举值转换为枚举名
const char* get_level_name(level level_name){
    
    switch(level_name){
        case ACTION_LOGIN:
            return "ACTION_LOGIN:";
        case ACTION_INFO:
            return "ACTION_INFO:";
        case ACTION_SERVER:
            return "ACTION_SERVER:";
        default:
            return "ACTION_DEFAULT:";
    }
}

//转换大端IP为点分十进制IP
const char* get_dottedDecimal_ip(const User* user){
    return  (inet_ntoa(user->clientaddr.sin_addr));
}

//根据函数返回值界定成功失败
const char* get_result(int func_result){
    
}


//拼接消息
const char* create_message(const char* time_now, 
    const char* dottedDecimal_ip,const msg* message){

    char* message_in_queue =(char*) malloc(512*sizeof(char));
    if (message_in_queue == NULL) {
        error(1,errno,"malloc in create_message\n");
    } 
    strcpy(message_in_queue, time_now);
    strcat(message_in_queue, " ");
    strcat(message_in_queue, dottedDecimal_ip);
    strcat(message_in_queue, " ");
    strcat(message_in_queue, get_level_name(message->log_level));
    strcat(message_in_queue, " ");
    strcat(message_in_queue, message->msg_body);

    return message_in_queue;
}


//初始化日志消息队列
mqd_t init_message_queue(const char *queue_name, struct mq_attr *attr) {
    mqd_t mqdes = mq_open(queue_name, O_CREAT | O_RDONLY, 0644, attr);
    if (mqdes == (mqd_t) -1) {
        perror("mq_open");
        return (mqd_t) -1;
    }
    return mqdes;
}


//向日志消息队列写入消息
int write_to_queue(mqd_t mqdes, const char* message) {

    if (mq_send(mqdes, message, strlen(message)+1, 0) == -1) {
        perror("mq_send");
        return -1;
    }
    return 0;
}

//从日志消息队列中取出消息
int read_from_queue_and_log(mqd_t mqdes, const char *log_file_path) {
    char buffer[MAX_MSG_SIZE];
    struct mq_attr attr;
    if (mq_getattr(mqdes, &attr) == -1) {
        perror("mq_getattr");
        return -1;
    }

    ssize_t num_read = mq_receive(mqdes, buffer, attr.mq_msgsize, NULL);
    if (num_read == -1) {
        perror("mq_receive");
        return -1;
    }

    FILE *log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        perror("fopen");
        return -1;
    }

    if (fwrite(buffer, sizeof(char), num_read, log_file) < num_read) {
        perror("fwrite");
        fclose(log_file);
        return -1;
    }

    fclose(log_file);
    return 0;
}
