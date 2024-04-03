#ifndef _OPTIONS_LINKEDLIST_H
#define _OPTIONS_LINKEDLIST_H

#define OPTION_NAME_MAX_SIZE 40

typedef struct ListNode
{
    char name[OPTION_NAME_MAX_SIZE];
    volatile int value;
    struct ListNode *next;
} ListNode;

extern ListNode options_head;

void insert_all_option();
int read_one_option(char *name);

#endif
