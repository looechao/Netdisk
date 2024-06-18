#include "linked_list.h"
#include "user.h"

// 创建新的链表节点
ListNode* createNode(void * val) {
    ListNode *newNode = (ListNode*)malloc(sizeof(ListNode));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->val = val;
    newNode->next = NULL;
    return newNode;
}

// 在链表末尾添加元素
void appendNode(ListNode **head, void *val) {
    ListNode *newNode = createNode(val);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    ListNode *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

// 删除链表中值为target的节点（假设只删除一个）
void deleteNode(ListNode **head, void * target) {
    if (*head == NULL) return;

    if ((*head)->val == target) {
        ListNode *temp = *head;
        *head = (*head)->next;
        free(temp);
        return;
    }

    ListNode *current = *head;
    while (current->next != NULL && current->next->val != target) {
        current = current->next;
    }

    if (current->next != NULL) {
        ListNode *temp = current->next;
        current->next = current->next->next;
        free(temp);
    }
}

// 删除链表中值为peerfd的节点（假设只删除一个）
void deleteNode2(ListNode **head, int peerfd)
{
    if (*head == NULL) return;
    user_t * user = (user_t*)((*head)->val);

    if (user->sockfd == peerfd) {
        ListNode *temp = *head;
        *head = (*head)->next;
        free(temp);
        return;
    }

    ListNode *current = *head;
    while (current->next != NULL) {
        if(((user_t*)current->next->val)->sockfd != peerfd) {
            current = current->next;
        }
    }

    if (current->next != NULL) {
        ListNode *temp = current->next;
        current->next = current->next->next;
        free(temp);
    }
}

// 释放链表内存
void freeList(ListNode *head) {
    ListNode *current = head;
    while (current != NULL) {
        ListNode *temp = current;
        current = current->next;
        free(temp);
    }
}


