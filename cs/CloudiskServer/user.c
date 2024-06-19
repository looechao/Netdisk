#include "user.h"
#include "thread_pool.h"
#include <stdio.h>
#include <string.h>
#include <shadow.h>
#include "log.c"
extern  log_LockFn my_lock_func;

static void get_setting(char *setting,char *passwd)
{
    int i,j;
    //取出salt,i 记录密码字符下标，j记录$出现次数
    for(i = 0,j = 0; passwd[i] && j != 4; ++i) {
        if(passwd[i] == '$')
            ++j;
    }
    strncpy(setting, passwd, i);
}

void username_check(user_t * user)
{
    printf("username_check.\n");
    train_t t;
    int ret;
    memset(&t, 0, sizeof(t));
    struct spwd * sp = getspnam(user->name);
    if(sp == NULL) {// 用户不存在的情况下
        t.len = 0;
        t.type = TASK_LOGIN_USERNAME_ERROR;
        t.file_size = 0;
        ret = sendn(user->sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size));
        write_log("用户不存在", "error", my_lock_func);
        printf("用户不存在\n");
        return;
    }
    printf("用户存在\n");
    //用户存在的情况下
    char setting[100] = {0};
    //保存加密密文
    strcpy(user->encrypted, sp->sp_pwdp);
    //提取setting
    get_setting(setting, sp->sp_pwdp);
    t.len = strlen(setting);
    t.type = TASK_LOGIN_USERNAME_OK;
    t.file_size = 0;
    strncpy(t.buff, setting, t.len);
    //发送setting
    ret = sendn(user->sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size) + t.len);
    printf("check1 send %d bytes.\n", ret);
}

void passwd_check(user_t * user, const char * encrypted)
{
    /* printf("passwd_check.\n"); */
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    if(strcmp(user->encrypted, encrypted) == 0) {
        //登录成功
        user->status = STATUS_LOGIN;//更新用户登录成功的状态
        t.type = TASK_LOGIN_PASSWD_OK;
        t.len = strlen("/server/$ ");// 暂定将 /server/ 作为pwd传递给client
        strcpy(t.buff, "/server/$ ");
        t.file_size = 0;
        ret = sendn(user->sockfd, &t, 12 + t.len);
        printf("Login success.\n");
    } else {
        //登录失败, 密码错误
        t.type = TASK_LOGIN_PASSWD_ERROR;
        write_log("密码错误", "error", my_lock_func);
        printf("登录失败\n");
        printf("Login failed.\n");
        ret = sendn(user->sockfd, &t, 8);
    }
    printf("check2 send %d bytes.\n", ret);
    return;
}
