#ifndef __DATABASES_H
#define __DATABASES_H

typedef struct {
    char parent_name[128];   // 父级目录名称，NULL 表示家目录
    char file_name[128];     // 要增加或删除的文件和目录
    int ower_id;
    char sha1[64];
    int filesize;
    char type;      // d 是目录， f是文件
} file_table;

typedef struct {
    int user_id;
    char user_name[128];
    char crytpasswd[128];
    char pwd[128];
} user_table;

// (成功返回 0， 失败返回 -1)
// 增加目录或上传文件
int add_file_table(file_table* ptable);

// 删除目录或文件
int sub_file_table(file_table* ptable);

// 要查找的目录或文件
int select_file_table(file_table* ptable); 

// 增加用户
int add_user_table(user_table* ptable);

// 查找用户
int select_user_table(user_table* ptable);
#endif
