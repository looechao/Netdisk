#include "client.h"

static int username_send(int sockfd, train_t *t);
static int passwd_send(int sockfd, train_t *t);

int login(int sockfd)
{
    train_t t;
    memset(&t, 0, sizeof(t));
    username_send(sockfd, &t);
    passwd_send(sockfd, &t);
    return 0;
}

static int username_send(int sockfd, train_t *pt)
{
    printf("username_send.\n");
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        printf("输入用户名：\n");
        char user[20]= {0};
        int ret = read(STDIN_FILENO, user, sizeof(user));
        user[ret - 1] = '\0';
        t.len = strlen(user);
        t.type = TASK_LOGIN_USERNAME;
        t.file_size = 0;
        strncpy(t.buff, user, t.len);
        ret = sendn(sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size) + t.len);
        printf("username_send %d bytes.\n", ret);

        //接收信息
        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, sizeof(t.len));
        printf("收到回复长度 %d\n", t.len);
        ret = recvn(sockfd, &t.type, sizeof(t.type));
        ret = recvn(sockfd, &t.file_size, sizeof(t.file_size));
        if(t.type == TASK_LOGIN_USERNAME_ERROR) {
            //无效用户名, 重新输入
            printf("用户名不存在\n");
            continue;
        }
        //用户名正确，读取setting
        ret = recvn(sockfd, t.buff, t.len);
        break;
    }
    memcpy(pt, &t, sizeof(t));
    return 0;
}

static int passwd_send(int sockfd, train_t * pt)
{
    printf("passwd_send.\n");
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        char * passwd = getpass("请输入密码：\n");
        /* printf("password: %s\n", passwd); */
        char * encrytped = crypt(passwd, pt->buff);
        /* printf("encrytped: %s\n", encrytped); */
        t.len = strlen(encrytped);
        t.type = TASK_LOGIN_PASSWD;
        t.file_size = pt->file_size;
        strncpy(t.buff, encrytped, t.len);
        ret = sendn(sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size)  + t.len);
        /* printf("passwd_send %d bytes.\n", ret); */

        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, sizeof(t.len));
        /* printf("2 length: %d\n", t.len); */
        ret = recvn(sockfd, &t.type, sizeof(t.type));
        ret = recvn(sockfd, &t.file_size, sizeof(t.file_size));
        if(t.type == TASK_LOGIN_PASSWD_ERROR) {
            //密码不正确
            printf("sorry, password is not correct.\n");
            continue;
        } else {
            //ret = recvn(sockfd, t.buff, t.len);
            printf("Login Success.\n");
            printf("please input a command.\n");
            //fprintf(stderr, "%s", t.buff);
            break;
        } 
    }
    return 0;
}
//其作用：确定接收len字节的数据
int recvn(int sockfd, void * buff, int len)
{
    int left = len;//还剩下多少个字节需要接收
    char * pbuf = (char*)buff;
    int ret = -1;
    while(left > 0) {
        ret = recv(sockfd, pbuf, left, 0);
        if(ret == 0) {
            break;
        } else if(ret < 0) {
            perror("recv");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    //当退出while循环时，left的值等于0
    return len - left;
}

//作用: 确定发送len字节的数据
int sendn(int sockfd, const void * buff, int len)
{
    int left = len;
    const char * pbuf = (char*) buff;
    int ret = -1;
    while(left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        if(ret < 0) {
            perror("send");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    return len - left;
}
