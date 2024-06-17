#include "client.h"
#include <string.h>
#define MAXLINE 1024 // 最大输入长度

CmdType get_type (train_t t) {
    if(strcmp(t.buff, "pwd") == 0) {
        return CMD_TYPE_PWD;
    }
    else if (strcmp(t.buff, "ls") == 0) {
        return CMD_TYPE_LS;
    }
    else if (strcmp(t.buff, "cd") == 0) {
        return CMD_TYPE_CD;
    }
    else if (strcmp(t.buff, "mkdir") == 0) {
        return CMD_TYPE_MKDIR;
    }
    else if (strcmp(t.buff, "rmdir") == 0) {
        return CMD_TYPE_RMDIR;
    }
    else if (strcmp(t.buff, "puts") == 0) {
        return CMD_TYPE_PUTS;
    }
    else if (strcmp(t.buff, "gets") == 0) {
        return CMD_TYPE_GETS;
    }
    else {
        return CMD_TYPE_NOTCMD;
    }
}

// 解析命令行输入，分离命令和参数
int parse_input(char* str, int peerfd) {
    char* args;
    args = strtok(str, " \n");
    
    train_t t;
    memset(&t, 0, sizeof(t));
    char* input;
    char* cnt;
    while ((cnt = strtok(NULL, " \n")) != NULL) {
        strcat(input, cnt);
    }
    
    strcpy(t.buff, input);
    t.len = strlen(input);
    t.type = get_type(t);
    send(peerfd, &t, sizeof(t) + t.len, 0); 
    
    return 0;
}

int prase(int peerfd) {
    char str[MAXLINE];
    for(;;) {
        printf(">> "); // 提示符
        fgets(str, MAXLINE, stdin); // 从stdin读取命令

        if (strcmp(str, "exit\n") == 0) { // 检查是否为exit命令
            break; // 退出循环
        }
        parse_input(str, peerfd);
    }
    return 0;
}
