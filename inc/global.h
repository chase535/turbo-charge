#ifndef _GLOBAL_H
#define _GLOBAL_H

#define BYPASS_CHARGE_CURRENT "500000"
#define APP_PACKAGE_NAME_MAX_SIZE 100

extern char option_file[];
extern char bypass_charge_file[];
extern char temp_sensors[][15];
extern char temp_sensor_quantity;

void free_malloc_memory(char ***addr, int num);

#endif
