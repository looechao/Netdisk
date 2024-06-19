
#include "client.h"

void cdCommand(int sockfd) {
    char is_true;
    char buf[4096] = { 0 };

    ssize_t rrbytes = recv(sockfd, &is_true, sizeof(char), MSG_WAITALL);
    printf("is_true = %c\n", is_true);

    rrbytes = recv(sockfd, buf, sizeof(buf), 0);
    printf("len = %ld\n", rrbytes);

    if (is_true == '1') {
        printf("%s\n", buf);
    }
    /* else if (is_true == '0') { */
    /*     printf("%s\n", buf); */
    /* } */
}

void rmdirCommand(int sockfd) {
    char is_true;
    char buf[4096] = { 0 };

    ssize_t rrbytes = recv(sockfd, &is_true, sizeof(char), MSG_WAITALL);
    printf("is_true = %c\n", is_true);

    rrbytes = recv(sockfd, buf, sizeof(buf), 0);
    printf("len = %ld\n", rrbytes);

    if (is_true == '1') {
        printf("%s\n", buf);
    }
    else if (is_true == '0') {
        printf("%s\n", buf);
    }
}
