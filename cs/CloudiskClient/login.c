#include "client.h"

int send_usename(int peerfd) {
    char* usename;
    usename = getpass("请输入用户名:");

    train_t t;
    memset(&t, 0, sizeof(t));

    t.len = strlen(usename);
    t.type = CMD_TYPE_USERNAME;
    strcpy(t.buff, usename);
    send(peerfd, &t, sizeof(t) + t.len, 0);
}

int send_passwd(int peerfd) {
    char* passwd;
    passwd = getpass("请输入密码");

    train_t t;
    memset(&t, 0, sizeof(t));

    t.len = strlen(passwd);
    t.type = CMD_TYPE_PASSWD;
    strcmp(t.buff, passwd);
    send(peerfd, &t, sizeof(t) + t.len, 0);
}
