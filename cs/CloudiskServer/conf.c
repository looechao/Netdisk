#include "thread_pool.h"

int read_conf(char *conf[],int n);

int get_conf(HashTable *hash){
    
    char *temp;
    char *key;
    //读取配置文件
    FILE *fd=fopen("./conf/server.conf","r");
    if(fd == NULL){
        return -1;
    }

    // 从文件读取数据分割 放入哈希表
    char buffer[4096]={0};
    int i=0;
    while (fgets(buffer, sizeof(buffer), fd) != NULL) {
        
        if(i==4096){
            fprintf(stderr,"conf big\n");
            return -1;
        }
        //分割配置文件
        key=strtok(buffer,"=");
        // 循环输出每个子串

        temp=strtok(NULL,"\n\0");
        
        char *value=(char *)calloc(1,strlen(temp)+1);
        strcpy(value,temp);
        printf("key=%s value=%s\n",key,value);
        //插入哈希表 0-key,1-value;
        insert(hash,key,value);
    
    }
    fclose(fd);   
    return 0;
}


