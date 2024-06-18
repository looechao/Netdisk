#include "client.h"

int main() {
    //连接服务器
     // int sockfd = my_connect("127.0.0.1", "8080");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);                 
     ERROR_CHECK(sockfd, -1, "sockfd");                           
                                                                  
    struct sockaddr_in addr;                                      
    int ret = inet_aton("127.0.0.1", &addr.sin_addr);                      
    ERROR_CHECK(ret, -1, "inet_aton");                            
    addr.sin_port = htons(atoi("8080"));                            
    addr.sin_family=AF_INET;                                      
                                                                  
    ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    ERROR_CHECK(ret, -1, "connect");                              
                                                                  
    //函数在login.c
    login(sockfd);

    char str[MAXLINE];
    for (; ; ) {
        printf(">> "); // 提示符
        fgets(str, MAXLINE, stdin); // 从stdin读取命令

        if (strcmp(str, "exit\n") == 0) { // 检查是否为exit命令
            break; // 退出循环
        }
        
        // 解析 并 发送命令
        CmdType type= parse_input(str, sockfd);
        
        // 接收服务器返回的信息 #所有人自己创建.c文件，如：gets.c文件存放 getsCommand(sockfd);   
        switch(type){
        case CMD_TYPE_PWD:  
            //pwdCommand(sockfd);   
            break;
        case CMD_TYPE_CD:
            //cdCommand(sockfd);    
            break;
        case CMD_TYPE_LS:
            //lsCommand(sockfd);    
            break;
        case CMD_TYPE_MKDIR:
            //mkdirCommand(sockfd);  
            break;
        case CMD_TYPE_RMDIR:
            //rmdirCommand(sockfd);  
            break;
        case CMD_TYPE_NOTCMD:
            //notCommand(sockfd);   
            break;
        case CMD_TYPE_PUTS:
            //putsCommand(sockfd);   
            break;
        case CMD_TYPE_GETS:
            getsCommand(sockfd);   
            break;
        default:
            break;
        }
    }
    return 0;
}
