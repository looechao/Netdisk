#include "thread_pool.h"
#include "log.h"

// cd 栈，用来解析路径字符串
typedef struct stack_node {
    char dir_name[1024];
    struct stack_node* next;
}stack_node; 

typedef struct {
    stack_node* top;
    int size;
}dir_stack;

// 入栈
void push_dir_stack(dir_stack* pstack, char* dir_name) {
    stack_node* node = (stack_node*)calloc(1, sizeof(stack_node));
    if (!node) {
        fprintf(stderr, "Calloc dir stack node failed!\n");
        exit(EXIT_FAILURE);
    }
    strcpy(node->dir_name, dir_name);
    node->next = pstack->top;
    pstack->top = node;
    pstack->size++;
}

// 出栈
void pop_dir_stack(dir_stack* pstack) {
    if (pstack->size == 0) {
        return;
    }

    stack_node* temp = pstack->top;
    pstack->top = pstack->top->next;

    free(temp);
    pstack->size--;

    return;
}

// 释放栈空间
void destroy_stack(dir_stack* pstack) {
    while(pstack->top != NULL) {
        stack_node* temp = pstack->top;
        pstack->top = pstack->top->next;
        free(temp);
    } 
}

// 分析是目录还是文件，返回值：目录 1， 普通文件 2， 无效路径 3
int is_dir(char* dir_path) {
    struct stat statbuff;
    if (stat(dir_path, &statbuff) == 0) {

        if (S_ISDIR(statbuff.st_mode)) {
            return 1;
        }
        else if (S_ISREG(statbuff.st_mode)) {
            return 2;
        }
    }
    return -1;
}

// 分析客户端传的路径，防止越界访问
void parse_path(task_t* task, char* curr_dir, char* newfile) {
    char home_dir[1024] = "./User";
    // 初始化栈
    dir_stack user_dir_stack;
    user_dir_stack.size = 0;
    user_dir_stack.top = NULL;
    
    // 绝对路径
    bool is_abs_path = false;
    if (task->data[0] == '/') {
        strcat(home_dir, task->data);
        strcpy(curr_dir, home_dir);
        is_abs_path = true;
    }

    // 将当前路径入栈
    char* token = strtok(curr_dir, "/\n");
    while (token != NULL) {
        push_dir_stack(&user_dir_stack, token);
        token = strtok(NULL, "/\n");
    }
    
    // 解析 task->data , 并入栈和出栈
    token = strtok(task->data, "/\n");                
    while (token != NULL) {                     
        if (strcmp(token, "..") == 0) {
            pop_dir_stack(&user_dir_stack);
        }
        else if (strcmp(token, ".") == 0) {
            token = strtok(NULL, "/\n");            
            continue;
        }
        else if (is_abs_path == false) {
            push_dir_stack(&user_dir_stack, token);     
        }

        token = strtok(NULL, "/\n");            
    }                                           

    while (user_dir_stack.size != 0) {
        char temp[4096] = { 0 };
        char* token = user_dir_stack.top->dir_name;
        sprintf(temp, "%s/%s", token, newfile);

        pop_dir_stack(&user_dir_stack); 
        strcpy(newfile, temp);
    }

    if (newfile[0] == '\0' || strcmp(newfile, "./") == 0) {
        strcpy(newfile, home_dir);
    }
}

// 删除第一个字符
void removeFirstChar(char *str) {
    while (*str != '\0') {
        *str = *(str + 1);
        str++;
    }
}

//每一个具体任务的执行，交给一个成员来实现

/* int cdCommand(task_t * task, msg* message) */
void cdCommand(task_t* task)
{
    printf("execute cd command.\n");
    // 存储操作信息
    char error_msg[4096] = { 0 };

    // 模拟用户根目录
    char home_dir[1024] = "./User";
    char current_dir[1024] = { 0 };
    strcpy(current_dir, client_users[task->peerfd].directory_address);
    char last_dir[1024] = "/User";

    // 暂存这次目录路径
    char last_filename[1024] = { 0 };
    strcpy(last_filename, current_dir);

    // 解析命令
    // cd 家目录
    // 使用 strrchr 查找字符 '\n' 最后一次出现的位置  
    char* last_post = strrchr(task->data, '\n');      
    if (last_post != NULL) {                         
        *last_post = '\0';                           
    }                                                

    if (strlen(task->data) > 1) {
        if (task->data[0] == '~') {
            removeFirstChar(task->data);   
            strcpy(current_dir, home_dir);
        }
        
        char newfile[1024] = { 0 };
        parse_path(task, current_dir, newfile);
        int ret = is_dir(newfile);
        if (ret == -1) {
            // 没有这个目录或文件
            sprintf(error_msg, "%s is not existing.\n", task->data); 

            char is_true = '1';
            send(task->peerfd, &is_true, sizeof(is_true), 0);
            int len = strlen(error_msg);
            sendn(task->peerfd, error_msg, len);

            return;
        }

        // 目录
        else if(ret == 1) {
            // 更新用户现在目录
            strcpy(current_dir, newfile);

        }
        // 普通文件
        else if (ret == 2) {
            sprintf(error_msg, "%s is a file and is not a dir.\n", task->data); 

            char is_true = '1';
            send(task->peerfd, &is_true, sizeof(is_true), 0);
            int len = strlen(error_msg);
            sendn(task->peerfd, error_msg, len);

            return;
        }
    }

    if (strcmp(task->data, "") == 0 || strcmp(task->data, "~") == 0) {
        strcpy(current_dir, home_dir);
    }
    // cd 上次目录
    else if (strcmp(task->data, "-") == 0) {
        char temp[1024] = { 0 };
        strcpy(temp, current_dir);
        strcpy(current_dir, last_dir);
        strcpy(last_dir, temp);
    }
    // else {
    //      // 相对路径
    //      if (strcmp(current_dir, "/") != 0) {
    //          // 拼接 / 在目录后
    //          strcat(current_dir, "/");
    //      }
    // }

    strcpy(client_users[task->peerfd].directory_address, current_dir);
    sprintf(error_msg, "Succeeded in switching directory %s\n", current_dir); 

    char is_true = '1';
    send(task->peerfd, &is_true, sizeof(is_true), 0);
    int len = strlen(error_msg);
    sendn(task->peerfd, error_msg, len);

    return;
}

void rmdirCommand(task_t * task)
{
    printf("execute rmdir command.\n");
    // 错误消息
    char error_msg[4096] = { 0 };

    // 模拟用户根目录
    char curr_dir[128] ;
    strcpy(curr_dir, client_users[task->peerfd].directory_address);

    char newfile[1024] = { 0 };
    parse_path(task, curr_dir, newfile);

    // 使用 strrchr 查找字符 '/' 最后一次出现的位置  
    char* last_post = strrchr(newfile, '/');      
    if (last_post != NULL) {                         
        *last_post = '\0';                           
    }

    // 判断是否是一个目录
    int ret = is_dir(newfile);
    if (ret == 2) {
        if (unlink(newfile) == -1) {
            sprintf(error_msg, "unlink %s failed.\n", task->data); 

            char is_true = '1';
            send(task->peerfd, &is_true, sizeof(is_true), 0);
            int len = strlen(error_msg);
            sendn(task->peerfd, error_msg, len);

            return;
        }
        sprintf(error_msg, "rm %s succese.\n", task->data);

        char is_true = '0';
        send(task->peerfd, &is_true, sizeof(is_true), 0);

        int len = strlen(error_msg);
        sendn(task->peerfd, error_msg, len);
        
        return;
    }
    else if (ret == 1) {
        ret = rmdir(newfile);
        if (ret == -1) {
            sprintf(error_msg, "rm %s is an empty directory.\n", task->data);

            char is_true = '1';
            send(task->peerfd, &is_true, sizeof(is_true), 0);

            int len = strlen(error_msg);
            sendn(task->peerfd, error_msg, len);

            return;
        }

        sprintf(error_msg, "rmdir %s succese.\n", task->data);

        char is_true = '0';
        send(task->peerfd, &is_true, sizeof(is_true), 0);

        int len = strlen(error_msg);
        sendn(task->peerfd, error_msg, len);

    }
    else if (ret == -1) {
        sprintf(error_msg, "%s is not existing.\n", task->data);         

        char is_true = '1';
        send(task->peerfd, &is_true, sizeof(is_true), 0);

        int len = strlen(error_msg);
        sendn(task->peerfd, error_msg, len);

        return;
    }

    return;
}