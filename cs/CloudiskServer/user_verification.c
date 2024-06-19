#include "thread_pool.h"
#include "user.h"
#include "linked_list.h"

extern ListNode * userList;

void username_validation(task_t * task) {
    printf("username_validation.\n");
    ListNode * pNode = userList;
    while(pNode != NULL) {
        user_t * user = (user_t *)pNode->val;
        if(user->sockfd == task->peerfd) {
            //拷贝用户名
            strcpy(user->name, task->data);
            username_check(user);
            return;
        } 
        pNode = pNode->next;
    }
}

void password_validation(task_t * task) {
    printf("password_validation.\n");
    ListNode * pNode = userList;
    while(pNode != NULL) {
        user_t * user = (user_t *)pNode->val;
        if(user->sockfd == task->peerfd) {
            //拷贝加密密文
            passwd_check(user, task->data);
            return;
        }
        pNode = pNode->next;
    }
}


































































