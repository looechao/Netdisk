#include "thread_pool.h"
#include <shadow.h>
#include <unistd.h>

char encrypted[128];

void get_salt(char* salt, char* passwd) {
    int i, j;
    // 取出salt, i记录密码字符下标，j记录$出现次数
    for (i = 0, j = 0; passwd[i] && j != 4; ++i) {
        if (passwd[i] == '$')
            ++j;
    }
    strncpy(salt, passwd, i - 1);
}

// 用户名错误断开连接
void error_quit(int sockfd) {
    //发送用户名、密码，错误，并关闭套接字
    int ret = 0;
    sendn(sockfd, &ret, sizeof(ret));
    //关闭套接字
}

void username_validation(task_t * task){
    struct spwd* sp;
    char salt[512] = {0};
    if ((sp = getspnam(task->data)) == NULL) {
        // 用户名错误断开连接
        error_quit(task->peerfd);
        return;
    }
    memset(&salt, 0, sizeof(salt));
    get_salt(salt, sp->sp_pwdp);

    train_t t;
    memset(&t, 0, sizeof(t));
    t.len = strlen(salt);
    t.type = CMD_TYPE_PASSWD;
    t.file_size = 0;
    strcpy(t.buff, salt);

    // 发送盐值
    send(task->peerfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size) + t.len, 0);

    // 复制暗码
    memset(&encrypted, 0, sizeof(encrypted));
    strcpy(encrypted, sp->sp_pwdp);
}

void password_validation(task_t * task){
    if (strcmp(encrypted, task->data) == 0) {
        //发送登录成功
        int ret = 1;
        sendn(task->peerfd,&ret,sizeof(ret));
    }
    else {
        // 失败 断开连接 ......
        error_quit(task->peerfd); 
    }

}
