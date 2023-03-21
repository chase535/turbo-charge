#include "global.h"

int opt_new[OPTION_QUANTITY]={0,1,0,50000000,15,95,80,52,2000000,45,5};
char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
char option_file[]="/data/adb/turbo-charge/option.txt";
char options[OPTION_QUANTITY][40]={"STEP_CHARGING_DISABLED","TEMP_CTRL","POWER_CTRL","CURRENT_MAX",
                                    "STEP_CHARGING_DISABLED_THRESHOLD","CHARGE_STOP","CHARGE_START",
                                    "TEMP_MAX","HIGHEST_TEMP_CURRENT","RECHARGE_TEMP","CYCLE_TIME"};
char temp_sensors[12][15]={"lcd_therm","conn_therm","modem_therm","wifi_therm","quiet_therm","mtktsbtsnrpa",
                            "mtktsbtsmdpa","mtktsAP","modem-0-usr","modem1_wifi","ddr-usr","cwlan-usr"};
