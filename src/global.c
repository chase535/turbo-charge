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
