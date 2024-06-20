#ifndef __DATABASES_H
#define __DATABASES_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include "log.h"

#define MYSQL_STMT_ERROR_CHECK(ret, stmt) { \
    if (ret) { \
        fprintf(stderr,"(%d, %s)\n", \
               mysql_stmt_errno(stmt), mysql_stmt_error(stmt)); \
        mysql_stmt_close(stmt); \
        return -1; \
    } \
}

typedef struct {
    int file_id;
    int parent_id;   // 父级目录名称，NULL 表示家目录
    char file_name[128];     // 要增加或删除的文件和目录
    int owner_id;
    char sha1[64];
    int filesize;
    char type;      // d 是目录， f是文件
    char tomb;      // y 是活着， n是死
} file_table;

typedef struct {
    int user_id;
    char user_name[128];
    char cryptpasswd[128];
    char pwd[128];
    int pwd_id;
    int sockfd;
} user_table;

extern user_table client_users[100];

// (成功返回 0， 失败返回 -1)
// 创建表
int create_table(void);

// 增加目录或上传文件(虚拟文件表)
int add_file_table(MYSQL* conn, file_table* ptable);

// 删除目录或文件(虚拟文件表)
int sub_file_table(MYSQL* conn, file_table* ptable);

// 要查找的目录或文件(虚拟文件表)
int select_file_table(MYSQL* mysql, file_table* ptable); 

// 根据 SHA-1 HASH 查找所有文件表(全部文件表)
int search_file(MYSQL* mysql, const char* sha1_hash);

// 增加用户(用户表)
int add_user_table(MYSQL* conn, user_table* ptable);

// 查找用户(用户表)
int select_user_table(MYSQL* mysql, user_table* ptable);
#endif
