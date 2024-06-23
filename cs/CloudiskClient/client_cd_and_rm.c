#include "client.h"

void cdCommand(int sockfd, char* username) {
    char is_true;
    char buf[4096] = { 0 };

    recv(sockfd, &is_true, sizeof(char), MSG_WAITALL);

    recv(sockfd, buf, sizeof(buf), 0);

    if (is_true == '1') {
        printf("%s\n", buf);
    }
    else if (is_true == '0') {
        strcpy(username, buf);
        printf("%s\n", buf);
    }
}

void lsCommand(int sockfd) {
    char buf[4096] = { 0 };

    recv(sockfd, buf, sizeof(buf), 0);
    
    printf("file_name\tfile_size\tfile_type\n");

    if (strcmp(buf, "-1") == 0) {
        return;
    }

    char* token = strtok(buf, " \n");
    int i = 1;
    
    while (token != NULL) {
        printf("%5s\t", token);

        if (strlen(token) < 8) {
            putchar('\t');
        }

        token = strtok(NULL, " \n");
        if (i % 3 == 0) {
            putchar('\n');
        }
        i++;
    }
}

void rmdirCommand(int sockfd) {
    char is_true;
    char buf[4096] = { 0 };

    recv(sockfd, &is_true, sizeof(char), MSG_WAITALL);

    recv(sockfd, buf, sizeof(buf), 0);

    if (is_true == '1') {
        printf("%s\n", buf);
    }
    // else if (is_true == '0') {
    //     printf("%s\n", buf);
    // }
}

void mkdirCommand(int sockfd) {
    char is_true;
    char buf[4096] = { 0 };

    recv(sockfd, &is_true, sizeof(char), MSG_WAITALL);

    recv(sockfd, buf, sizeof(buf), 0);

    if (is_true == '1') {
        printf("%s\n", buf);
    }
    // else if (is_true == '0') {
    //     printf("%s\n", buf);
    // }
}

void rmCommand(int sockfd) {
    char is_true;
    char buf[4096] = { 0 };

    recv(sockfd, &is_true, sizeof(char), MSG_WAITALL);

    recv(sockfd, buf, sizeof(buf), 0);

    if (is_true == '1') {
        printf("%s\n", buf);
    }
    // else if (is_true == '0') {
    //     printf("%s\n", buf);
    // }
}

void notCommand(void) {
    printf("Not command!\n"
           "Please enter 'ls' 'cd' 'pwd' 'mkdir' 'rmdir' 'rm' 'gets' 'puts' command.\n");
}


// 计算文件 SHA-1 哈希值
void file_sha1(int fd, char* hash_value)
{
    // 计算 SHA-1 值
    // 存放加密摘要上下文
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();;
    // 采用 SHA-1 加密算法
    const EVP_MD* md = EVP_sha1();;

    // 初始化
    EVP_DigestInit_ex(ctx, md, NULL);

    char buff[1024];    // 暂存读文件内容
    while (1) {
        memset(buff, 0, sizeof(buff));
        ssize_t rbytes = read(fd, buff, sizeof(buff));
        if (rbytes == 0) {
            break;
        }
        else if (rbytes == -1) {
            perror("read_file");
            exit(EXIT_FAILURE);
        }

        // 对每一段内容调用更新函数
        EVP_DigestUpdate(ctx, buff, rbytes);
    }

    // 存储解析文件哈希值
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len;   // 存放生产哈希值的长度
    EVP_DigestFinal_ex(ctx, hash, &len);

    // 存储 16 进制 hash 值
    char result[EVP_MAX_MD_SIZE * 2 + 1] = { 0 };
    for (unsigned int i = 0; i < len; ++i) {
        char frag[3] = { 0 };
        sprintf(frag, "%02x", hash[i]);
        strcat(result, frag);
    }
    strcpy(hash_value, result);
}
