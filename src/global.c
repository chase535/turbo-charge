#include "global.h"

#include "stdlib.h"
#include "string.h"

char option_file[]="/data/adb/turbo-charge/option.txt";
char bypass_charge_file[]="/data/adb/turbo-charge/bypass_charge.txt";
char temp_sensors[TEMP_SENSOR_QUANTITY][15]={"conn_therm", "modem_therm", "wifi_therm", "mtktsbtsnrpa", "lcd_therm", "quiet_therm",
                                            "mtktsbtsmdpa", "mtktsAP", "modem-0-usr", "modem1_wifi", "ddr-usr", "cwlan-usr"};

volatile char ForegroundAppName[100];

ListNode *options;

void insert_option(char *name, int value)
{
    ListNode *next,*tmp=options;
    next=(ListNode *)calloc(1,sizeof(ListNode));
    strncpy(next->name,name,40);
    next->value=value;
    next->next=NULL;
    while(tmp->next) tmp=tmp->next;
    tmp->next=next;
}

void node_init()
{
    options=(ListNode *)calloc(1,sizeof(ListNode));
    options->next=NULL;
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
