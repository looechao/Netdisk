#include "client.h"
#define MAXLINE 1024 // 最大输入长度

int check_files(train_t *command);
void splitString(const char * pstrs, char *tokens[], int max_tokens, int * pcount);
void freeStrs(char * pstrs[], int count);

//获取命令类型
CmdType get_type (char* args) {
    if(strcmp(args, "pwd") == 0) {
        return CMD_TYPE_PWD;
    }
    else if (strcmp(args, "ls") == 0) {
        return CMD_TYPE_LS;
    }
    else if (strcmp(args, "cd") == 0) {
        return CMD_TYPE_CD;
    }
    else if (strcmp(args, "mkdir") == 0) {
        return CMD_TYPE_MKDIR;
    }
    else if (strcmp(args, "rmdir") == 0) {
        return CMD_TYPE_RMDIR;
    }
    else if (strcmp(args, "puts") == 0) {
        return CMD_TYPE_PUTS;
    }
    else if (strcmp(args, "gets") == 0) {
        return CMD_TYPE_GETS;
    }
    else {
        return CMD_TYPE_NOTCMD;
    }
}

// 解析命令行输入，分离命令和参数
CmdType parse_input(char* input, int peerfd) {
    char * strs[10] = {0};
    int cnt = 0;
    //分解字符串
    splitString(input, strs, 10, &cnt);
    
    //解析命令
    train_t information;
    information.type = get_type(strs[0]);
    
    //假设最多两个命令
    //for(int i = 1; i < cnt; ++i) {
        //printf("strs[%d]: %s\n", i, strs[i]);
        information.len=strlen(strs[1]);
        strcpy(information.buff,strs[1]);
    //}

    //如果是gets命令，需要查看当前是否有这个文件
    if(information.type == CMD_TYPE_GETS){
        check_files(&information);   
    }

    //发送命令
    send(peerfd, &information, 
         sizeof(information.len)+sizeof(information.type)
         +sizeof(information.file_size)+ information.len
         , 0); 
    
    freeStrs(strs, cnt);
    return information.type;
}

int check_files(train_t *command){
     //默认存储文件夹./default_download
    char default_download[]="./default_download/";

    char full_path[1024];
    // 初始化 full_path 为 default_download 的内容
    strcpy(full_path, default_download);
    // 将 full_path 拼接到 full_path
    strcat(full_path, command->buff);

    
    struct stat file_stat;

    // 使用stat函数获取文件信息
    if (stat(full_path, &file_stat) == 0) {
        printf("文件 %s 存在,在后面继续下载\n", full_path);
        printf("文件大小: %ld 字节\n", file_stat.st_size);
        command->file_size=file_stat.st_size;
    } else {
        printf("文件 %s 没有，需重新下载\n", full_path);
        command->file_size=0;
    }
    
    return 1;
}

// 假设max_tokens是数组tokens的最大大小
// 在使用后，记得要释放空间
void splitString(const char * pstrs, char *tokens[], int max_tokens, int * pcount) {
    int token_count = 0;
    char *token = strtok((char *)pstrs, " "); // 使用空格作为分隔符

    while (token != NULL && token_count < max_tokens - 1) { // 保留一个位置给NULL终止符
        char * pstr = (char*)calloc(1, strlen(token) + 1);
        strcpy(pstr, token);
        tokens[token_count] = pstr;//保存申请的堆空间首地址
        token_count++;
        token = strtok(NULL, " "); // 继续获取下一个token
    }
    // 添加NULL终止符
    tokens[token_count] = NULL;
    *pcount= token_count;
}

void freeStrs(char * pstrs[], int count)
{
    for(int i = 0; i < count; ++i) {
        free(pstrs[i]);
    }
}

