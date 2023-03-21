#include "global.h"

struct option_struct options[OPTION_QUANTITY]={{"CYCLE_TIME",5},
                                                {"STEP_CHARGING_DISABLED",0},
                                                {"TEMP_CTRL",1},
                                                {"POWER_CTRL",0},
                                                {"CURRENT_MAX",50000000},
                                                {"STEP_CHARGING_DISABLED_THRESHOLD",15},
                                                {"CHARGE_STOP",95},
                                                {"CHARGE_START",80},
                                                {"TEMP_MAX",52},
                                                {"HIGHEST_TEMP_CURRENT",2000000},
                                                {"RECHARGE_TEMP",45}};

char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
char option_file[]="/data/adb/turbo-charge/option.txt";
char temp_sensors[12][15]={"lcd_therm","conn_therm","modem_therm","wifi_therm","quiet_therm","mtktsbtsnrpa",
                            "mtktsbtsmdpa","mtktsAP","modem-0-usr","modem1_wifi","ddr-usr","cwlan-usr"};
