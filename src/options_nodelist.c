#include "stdlib.h"
#include "string.h"

#include "options_nodelist.h"

ListNode *options_head;

void insert_option(char *name, int value)
{
    ListNode *next,*tmp=options_head;
    next=(ListNode *)calloc(1, sizeof(ListNode));
    strncpy(next->name, name, 40);
    next->value=value;
    next->next=NULL;
    while(tmp->next) tmp=tmp->next;
    tmp->next=next;
}

void options_nodelist_init()
{
    options_head=(ListNode *)calloc(1, sizeof(ListNode));
    options_head->next=NULL;
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
