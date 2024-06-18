#include "client.h"

int login(int sockfd){
    send_usename(sockfd);

    train_t t;

    memset(&t, 0, sizeof(t));
    recv(sockfd, &t.len, sizeof(t.len), 0);
    recv(sockfd, &t.type, sizeof(t.type), 0);
    recv(sockfd, &t.file_size, sizeof(t.file_size), 0);
    recv(sockfd, &t.buff, t.len, 0);

    if (t.type == CMD_TYPE_PASSWD) {
        send_passwd(sockfd, &t);
        int i = 0;
        recv(sockfd, &i, sizeof(int), 0);
    }
    else {
        printf("ERROR: 错误");
    }
    return 0;
}

int send_usename(int peerfd) {
    char* usename;
    usename = getpass("请输入用户名:");

    train_t t;
    memset(&t, 0, sizeof(t));

    t.len = strlen(usename);
    t.type = CMD_TYPE_USERNAME;
    strcpy(t.buff, usename);
    send(peerfd, &t, sizeof(t) + t.len, 0);

    return 0;
}

int send_passwd(int peerfd, train_t* t) {
    char* passwd;
    passwd = getpass("请输入密码");

    char* key;
    key = crypt(passwd, t->buff);
    train_t t_;
    memset(&t, 0, sizeof(t));

    t_.len = strlen(key);
    t_.type = CMD_TYPE_PASSWD;
    strcpy(t_.buff, key);
    send(peerfd, &t, sizeof(t) + t_.len, 0);

    return 0;
}
