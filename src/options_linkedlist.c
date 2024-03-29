#include <stdlib.h>
#include <string.h>

#include "my_malloc.h"
#include "options_linkedlist.h"

//存储配置的个数
unsigned char option_quantity=0;

//链表的头结点，只存储开始节点的地址
ListNode options_head;

//添加节点
void insert_option(char *name, int value)
{
    ListNode *next,*tmp=&options_head;
    next=(ListNode *)my_calloc(1, sizeof(ListNode));
    strncpy(next->name, name, OPTION_NAME_MAX_SIZE-1);
    next->value=value;
    next->next=NULL;
    while(tmp->next) tmp=tmp->next;
    tmp->next=next;
}

//添加配置并将所有配置预先设为初始值
void insert_all_option()
{
    insert_option("CYCLE_TIME", 1);
    insert_option("FORCE_TEMP", 1);
    insert_option("CURRENT_MAX", 50000000);
    insert_option("STEP_CHARGING_DISABLED", 0);
    insert_option("TEMP_CTRL", 1);
    insert_option("POWER_CTRL", 0);
    insert_option("STEP_CHARGING_DISABLED_THRESHOLD", 15);
    insert_option("CHARGE_STOP", 95);
    insert_option("CHARGE_START", 80);
    insert_option("TEMP_MAX", 52);
    insert_option("HIGHEST_TEMP_CURRENT", 2000000);
    insert_option("RECHARGE_TEMP", 45);
    insert_option("BYPASS_CHARGE", 0);
}

//检查节点的个数（配置的个数）
unsigned char check_option_quantity()
{
    unsigned char num=0;
    ListNode *tmp=&options_head;
    while((tmp=tmp->next) != NULL) num++;
    return num;
}

//初始化存储配置的链表
void options_linkedlist_init()
{
    insert_all_option();
    option_quantity=check_option_quantity();
}
