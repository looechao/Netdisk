#include "thread_pool.h"
/* #include "user.h" */
/* #include "linked_list.h" */
#include "databases.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "log.h"

extern  log_LockFn my_lock_func;
/* extern ListNode * userList; */
extern user_table client_users[100];

void register_username(task_t * task, MYSQL* conn) {
    printf("register username.\n");

    // 初始化用户表
    user_table u1;
    memset(&u1, 0, sizeof(u1));
    u1.sockfd = task->peerfd;
    strcpy(u1.user_name, task->data); 

    // 查表，检查用户是否已存在
    int ret = select_user_table(conn, &u1);

    // 用户已存在
    if (ret == 0) {
        train_t t;
        t.len = 0;
        t.type = TASK_REGISTER_USERNAME_ERROR;
        t.file_size = 0;
        sendn(u1.sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size));
        write_log("注册用户已存在", "error", my_lock_func);
        printf("注册用户已存在\n");
        return;
    }

    // 用户不存在，注册
    client_users[task->peerfd] = u1;
    // 生成随机盐值
    unsigned char salt[32];
    RAND_bytes(salt, sizeof(salt));
    char salt_hash[65];
    for (unsigned int i = 0; i < sizeof(salt); i++) {
        sprintf(&salt_hash[i * 2], "%02x", salt[i]);
    }
    /* salt[32] = '\0'; */
    strcpy(client_users[task->peerfd].salt, salt_hash);
   
    train_t t;
    memset(&t, 0, sizeof(t));
    strcpy(t.buff, salt_hash);
    t.len = strlen(t.buff);
    t.type = TASK_REGISTER_USERNAME_OK;
    t.file_size = 0;
    sendn(u1.sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size) + t.len);
}

void register_password(task_t * task, MYSQL* conn) {
    printf("register password.\n");

    // 初始化用户表
    user_table u1;
    memset(&u1, 0, sizeof(u1));
    u1.sockfd = task->peerfd;
    strcpy(u1.user_name, client_users[task->peerfd].user_name); 
    strcpy(u1.salt, client_users[task->peerfd].salt);

    strcpy(u1.cryptpasswd, task->data);
    strcpy(u1.pwd, "~");

    // 添加用户
    int ret = add_user_table(conn, &u1);
    if (ret == -1) {
        train_t t;
        t.len = 0;
        t.type = TASK_REGISTER_ERROR;
        t.file_size = 0;
        sendn(u1.sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size));
        write_log("注册用户插入用户表失败", "error", my_lock_func);
        printf("注册用户插入用户表失败\n");
        return;
    }

    train_t t;
    memset(&t, 0, sizeof(t));
    t.len = strlen(t.buff);
    t.type = TASK_REGISTER_SUCESSES;
    t.file_size = 0;
    sendn(u1.sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size));

    write_log("注册用户成功", "info", my_lock_func);
    printf("注册用户成功\n");

    memset(&client_users[task->peerfd], 0, sizeof(user_table));
}

void login_username(task_t* task, MYSQL* conn) {
    printf("login username.\n");

    // 初始化用户表
    user_table u1;
    memset(&u1, 0, sizeof(u1));
    u1.sockfd = task->peerfd;
    strcpy(u1.user_name, task->data); 

    // 查表，检查用户是否存在
    int ret = select_user_table(conn, &u1);

    // 用户不存在
    if (ret == -1) {
        train_t t;
        t.len = 0;
        t.type = TASK_REGISTER_USERNAME_ERROR;
        t.file_size = 0;
        sendn(u1.sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size));
        write_log("用户不存在", "error", my_lock_func);
        printf("用户不存在\n");
        return;
    }

    // 用户存在，发送盐值
    train_t t;
    memset(&t, 0, sizeof(t));
    client_users[task->peerfd] = u1;
    // 查表得盐值
    strcpy(t.buff, u1.salt);
    t.len = strlen(t.buff);
    t.type = TASK_LOGIN_USERNAME_OK;
    t.file_size = 0;
    sendn(u1.sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size) + t.len);
    printf("登录结束\n");
}

void login_password(task_t * task) {
    printf("login password.\n");

    if (strcmp(client_users[task->peerfd].cryptpasswd, task->data) == 0) {
        // 密码正确
        train_t t;
        memset(&t, 0, sizeof(t));
        t.len = strlen(t.buff);
        t.type = TASK_LOGIN_PASSWD_OK;
        t.file_size = 0;
        sendn(task->peerfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size) + t.len);

        write_log("用户登录成功", "info", my_lock_func);
        printf("用户登录成功\n");
        return;
    }

    // 密码错误
    train_t t;
    t.len = 0;
    t.type = TASK_LOGIN_PASSWD_ERROR;
    t.file_size = 0;
    sendn(task->peerfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size));
    write_log("用户登录失败", "error", my_lock_func);
    printf("用户登录失败\n");
}
