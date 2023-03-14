#include "global.h"

unsigned int opt_old[OPTION_QUANTITY]={0},opt_new[OPTION_QUANTITY]={0,1,0,50000000,15,95,80,52,2000000,45};
unsigned char tmp[5]={0};
char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
char option_file[]="/data/adb/turbo-charge/option.txt";
char options[OPTION_QUANTITY][40]={"STEP_CHARGING_DISABLED","TEMP_CTRL","POWER_CTRL","CURRENT_MAX",
                                    "STEP_CHARGING_DISABLED_THRESHOLD","CHARGE_STOP","CHARGE_START",
                                    "TEMP_MAX","HIGHEST_TEMP_CURRENT","RECHARGE_TEMP"};
