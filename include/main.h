#ifndef _TURBO_CHARGE_H
#define _TURBO_CHARGE_H

#define OPTION_QUANTITY 10
#define PRINTF_WITH_TIME_MAX_SIZE 400

typedef unsigned char uchar;
typedef unsigned int uint;

uint opt_old[OPTION_QUANTITY]={0},opt_new[OPTION_QUANTITY]={0,1,0,50000000,15,95,80,52,2000000,45};
uchar tmp[5]={0};
char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
char option_file[]="/data/adb/turbo-charge/option.txt";
char options[OPTION_QUANTITY][40]={"STEP_CHARGING_DISABLED","TEMP_CTRL","POWER_CTRL","CURRENT_MAX",
                                    "STEP_CHARGING_DISABLED_THRESHOLD","CHARGE_STOP","CHARGE_START",
                                    "TEMP_MAX","HIGHEST_TEMP_CURRENT","RECHARGE_TEMP"};

void free_celloc_memory(char ***addr, int num);
void line_feed(char *line);
void set_value(char *file, char *numb);
void set_array_value(char **file, int num, char *value);
void charge_value(char *i);
void check_read_file(char *file);
void step_charge_ctl(char *value);
void powel_ctl(void);
int list_dir(char *path, char ***ppp);

#endif
