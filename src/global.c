#include "global.h"

/* 
一些随时都可能修改的变量，单独拿出来以便于后续修改
在global.h头文件中还有一个关于旁路充电电流的宏定义
*/

char option_file[]="/data/adb/turbo-charge/option.txt";
char bypass_charge_file[]="/data/adb/turbo-charge/bypass_charge.txt";
char temp_sensors[][15]={"conn_therm", "modem_therm", "wifi_therm", "mtktsbtsnrpa", "lcd_therm", "quiet_therm",
                        "mtktsbtsmdpa", "mtktsAP", "modem-0-usr", "modem1_wifi", "ddr-usr", "cwlan-usr"};
char temp_sensor_quantity=sizeof(temp_sensors)/sizeof(temp_sensors[0]);

//完全释放动态申请的二级指针的内存
void free_malloc_memory(char ***addr, int num)
{
    if(addr != NULL && *addr != NULL)
    {
        if(!num) num=1;
        for(int i=0;i < num;i++)
        {
            if((*addr)[i] != NULL)
            {
                my_free((*addr)[i]);
                (*addr)[i]=NULL;
            }
        }
        my_free(*addr);
        *addr=NULL;
    }
}
