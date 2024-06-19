#ifndef __USER_H__
#define __USER_H__

typedef enum {
    STATUS_LOGOFF = 0,
    STATUS_LOGIN
}LoginStatus;

typedef struct {
    int sockfd;//套接字文件描述符
    LoginStatus status;//登录状态
    char name[20];//用户名(客户端传递过来的)
    char encrypted[100];//从/etc/shadow文件中获取的加密密文
    char pwd[128];//用户当前路径

}user_t;

void username_check(user_t * user);
void passwd_check(user_t * user, const char * encrypted);

#endif
