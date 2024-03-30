#ifndef _OPTIONS_LINKEDLIST_H
#define _OPTIONS_LINKEDLIST_H

#define OPTION_NAME_MAX_SIZE 40

struct ListNode
{
    char name[OPTION_NAME_MAX_SIZE];
    volatile int value;
    struct ListNode *next;
};

typedef struct ListNode ListNode;

extern unsigned char option_quantity;
extern ListNode options_head;

void options_linkedlist_init();

#endif
