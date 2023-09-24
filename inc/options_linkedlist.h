#ifndef _OPTIONS_NODELIST_H
#define _OPTIONS_NODELIST_H

struct ListNode
{
    char name[40];
    volatile int value;
    struct ListNode *next;
};

typedef struct ListNode ListNode;

extern char option_quantity;
extern ListNode options_head;

void options_nodelist_init();

#endif
