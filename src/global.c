#include "global.h"

char option_file[]="/data/adb/turbo-charge/option.txt";
char bypass_charge_file[]="/data/adb/turbo-charge/bypass_charge.txt";
char temp_sensors[TEMP_SENSOR_QUANTITY][15]={"conn_therm", "modem_therm", "wifi_therm", "mtktsbtsnrpa", "lcd_therm", "quiet_therm",
                                            "mtktsbtsmdpa", "mtktsAP", "modem-0-usr", "modem1_wifi", "ddr-usr", "cwlan-usr"};

volatile char ForegroundAppName[100];
volatile option_struct options[OPTION_QUANTITY]={{"CYCLE_TIME", 1},
                                                {"FORCE_TEMP", 1},
                                                {"CURRENT_MAX", 50000000},
                                                {"STEP_CHARGING_DISABLED", 0},
                                                {"TEMP_CTRL", 1},
                                                {"POWER_CTRL", 0},
                                                {"STEP_CHARGING_DISABLED_THRESHOLD", 15},
                                                {"CHARGE_STOP", 95},
                                                {"CHARGE_START", 80},
                                                {"TEMP_MAX", 52},
                                                {"HIGHEST_TEMP_CURRENT", 2000000},
                                                {"RECHARGE_TEMP", 45},
                                                {"BYPASS_CHARGE", 0}};
