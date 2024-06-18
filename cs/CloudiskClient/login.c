#include "client.h"

int login(int sockfd){

    int sign = -1;
    int cnt_send = 0;
    train_t t;
    while (1) {
        send_usename(sockfd);
    	memset(&t, 0, sizeof(t));
  		// 接收服务端发送的信息 
    	recv(sockfd, &t.len, sizeof(t.len), 0);
    	recv(sockfd, &t.type, sizeof(t.type), 0);
    	recv(sockfd, &t.file_size, sizeof(t.file_size), 0);
    	recv(sockfd, &t.buff, t.len, 0);    

        sign = atoi(t.buff);
        if (cnt_send == 3) {
            printf("用户名输入错误三次,退出!\n");
            return 0;
        }

        if (sign == -1) {
            ++cnt_send;
            printf("登录失败!\n");
            printf("用户名错误，请重新输入\n");
        }
        else {
            printf("登录成功!\n");
            break;
        }
    }

    cnt_send = 0;
    sign = -1;
    while (1) {
        send_passwd(sockfd, &t);    

        recv(sockfd, &sign, sizeof(int), 0);

        if (cnt_send == 3) {
            printf("密码输入错误三次，退出!\n");
            return 0;
        }
        if (sign == -1) {
            ++cnt_send;
            printf("登录失败!\n");
            printf("密码不匹配，请重新输入\n");
        }
        else {
            printf("登录成功!\n");
            break;
        }
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
    t.file_size = 0;
    strcpy(t.buff, usename);
    send(peerfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size) + t.len, 0);

    return 0;
}

int send_passwd(int peerfd, train_t* s) {
    char* passwd;
    passwd = getpass("请输入密码:");

    char* key;
    key = crypt(passwd, s->buff);
    train_t t;
    memset(&t, 0, sizeof(t));

    t.len = strlen(key);
    t.type = CMD_TYPE_PASSWD;
    t.file_size =0;
    strcpy(t.buff, key);
    send(peerfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size) + t.len, 0);

    return 0;
}
