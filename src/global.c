#include "global.h"

char option_file[]="/data/adb/turbo-charge/option.txt";
char bypass_charge_file[]="/data/adb/turbo-charge/bypass_charge.txt";
char temp_sensors[TEMP_SENSOR_QUANTITY][15]={"conn_therm", "modem_therm", "wifi_therm", "mtktsbtsnrpa", "lcd_therm", "quiet_therm",
                                            "mtktsbtsmdpa", "mtktsAP", "modem-0-usr", "modem1_wifi", "ddr-usr", "cwlan-usr"};

volatile char ForegroundAppName[100];
