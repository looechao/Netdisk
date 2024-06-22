#include "client.h"

static int login_username_send(int sockfd, train_t *t);
static int login_passwd_send(int sockfd, train_t *t);

static int register_username_send(int sockfd, train_t *t);
static int register_passwd_send(int sockfd, train_t *t);

// 盐值加密密码, 使用SHA-256算法哈希过的加密密码
void generate_encrypted_code(char* passwd, char* salt, char* cryptpasswd) {
    char salted_pass[255];
    strcpy(salted_pass, passwd);
    strcat(salted_pass, salt);

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, salted_pass, strlen(salted_pass));

    unsigned char value[EVP_MAX_MD_SIZE];
    unsigned int len;

    EVP_DigestFinal_ex(ctx, value, &len);
    EVP_MD_CTX_free(ctx);

    for (unsigned int i = 0; i < len; ++i) {
        sprintf(&cryptpasswd[i * 2], "%02x", value[i]);
    }
}

int login(int sockfd)
{
    train_t t;
    memset(&t, 0, sizeof(t));

    printf("请输入 [0]register 或 [1]login ：\n");
    while (1) {
        int num;
        scanf("%d", &num);
        if (num == 0) {
            register_username_send(sockfd, &t);
            /* printf("username_finish\n"); */
            register_passwd_send(sockfd, &t);
        }
        else if (num != 1) {
            puts("输入无效，请输入 [0]register 或 [1]login ：");
            continue;
        }
        break;
    }

    login_username_send(sockfd, &t);
    login_passwd_send(sockfd, &t);
    return 0;
}

static int register_username_send(int sockfd, train_t *t) {
    
    while (1) {
        printf("Enter username:\n");
        char user[20]= {0};
        int ret = read(STDIN_FILENO, user, sizeof(user));
        user[ret - 1] = '\0';

        // 发送用户名
        memset(t, 0, sizeof(*t));
        t->len = strlen(user);
        t->type = TASK_REGISTER_USERNAME;
        t->file_size = 0;
        strncpy(t->buff, user, t->len);
        ret = sendn(sockfd, t, sizeof(t->len) + sizeof(t->type) + sizeof(t->file_size) + t->len);
        /* printf("username_send %d bytes.\n", ret); */

        //接收信息
        memset(t, 0, sizeof(*t));
        ret = recvn(sockfd, &t->len, sizeof(t->len));
        /* printf("收到回复长度 %d\n", t->len); */
        ret = recvn(sockfd, &t->type, sizeof(t->type));
        ret = recvn(sockfd, &t->file_size, sizeof(t->file_size));
        if(t->type == TASK_REGISTER_USERNAME_ERROR) {
            //无效用户名, 重新输入
            printf("用户名已存在\n");
            continue;
        }
        //用户名，读取 salt
        /* puts("finish"); */
        ret = recvn(sockfd, t->buff, t->len);
        /* printf("salt %d bytes.\n", ret); */
        break;
    }
    return 0;
}

static int register_passwd_send(int sockfd, train_t* t)
{
    printf("passwd_send.\n");
    char* passwd;
    while (1) {
        passwd = getpass("Enter password：\n");
        char temp_passwd[20] = { 0 };
        strcpy(temp_passwd, passwd);
        passwd = getpass("Enter password again：\n");

        if (strcmp(passwd, temp_passwd) == 0) {
            break;
        }
        else {
            puts("Password inconsistency");
            continue;
        }
    }

    char cryptpasswd[255];
    generate_encrypted_code(passwd, t->buff, cryptpasswd);

    memset(t, 0, sizeof(*t));
    t->len = strlen(cryptpasswd);
    t->type = TASK_REGISTER_PASSWD;
    t->file_size = 0;
    strncpy(t->buff, cryptpasswd, t->len);
    int ret = sendn(sockfd, t, sizeof(t->len) + sizeof(t->type) + sizeof(t->file_size)  + t->len);
    /* printf("passwd_send %d bytes.\n", ret); */
    if(ret == -1){
        printf("send cryptpasswd failed!\n");
        exit(EXIT_FAILURE);
    }

    memset(t, 0, sizeof(*t));
    ret = recvn(sockfd, &t->len, sizeof(t->len));
    /* printf("2 length: %d\n", t.len); */
    ret = recvn(sockfd, &t->type, sizeof(t->type));
    ret = recvn(sockfd, &t->file_size, sizeof(t->file_size));
    if(t->type == TASK_REGISTER_ERROR) {
        //密码格式不正确
        printf("sorry, password is not correct.\n");
        exit(EXIT_FAILURE);
    } else {
        //ret = recvn(sockfd, t.buff, t.len);
        printf("Register Success.\n");
        //fprintf(stderr, "%s", t.buff);
    }
    return 0;
}

static int login_username_send(int sockfd, train_t *pt)
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

        //接收信息
        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, sizeof(t.len));
        /* printf("收到回复长度 %d\n", t.len); */
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

static int login_passwd_send(int sockfd, train_t * pt)
{
    printf("passwd_send.\n");
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        char * passwd = getpass("请输入密码：\n");

        char cryptpasswd[255];
        generate_encrypted_code(passwd, pt->buff, cryptpasswd);

        t.len = strlen(cryptpasswd);
        t.type = TASK_LOGIN_PASSWD;
        t.file_size = pt->file_size;
        strncpy(t.buff, cryptpasswd, t.len);
        ret = sendn(sockfd, &t, sizeof(t.len) + sizeof(t.type) + sizeof(t.file_size)  + t.len);
        /* printf("passwd_send %d bytes.\n", ret); */
        if(ret==-1){
            printf("send cryptpasswd failed!\n");
            exit(EXIT_FAILURE);
        }

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
