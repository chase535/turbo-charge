#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "unistd.h"
#include "time.h"
#include "regex.h"
#include "malloc.h"
#include "sys/types.h"
#include "sys/stat.h"

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

struct tm *get_utc8_time(void)
{
    time_t cur_time;
    struct tm *ptm;
    time(&cur_time);
    ptm=gmtime(&cur_time);
    ptm->tm_year+=1900;
    ptm->tm_mon+=1;
    ptm->tm_hour+=8;
    if(ptm->tm_hour > 23)
    {
        ptm->tm_hour-=24;
        ptm->tm_mday+=1;
        switch(ptm->tm_mon)
        {
            case 1: case 3: case 5: case 7: case 8: case 10: case 12:
                if(ptm->tm_mday > 31)
                {
                    ptm->tm_mday-=31;
                    ptm->tm_mon+=1;
                }
                break;
            case 2:
                if(((ptm->tm_year%4 == 0) && (ptm->tm_year%100 != 0)) || (ptm->tm_year%400 == 0))
                {
                    if(ptm->tm_mday > 29)
                    {
                        ptm->tm_mday-=29;
                        ptm->tm_mon+=1;
                    }
                }
                else
                {
                    if(ptm->tm_mday > 28)
                    {
                        ptm->tm_mday-=28;
                        ptm->tm_mon+=1;
                    }
                }
                break;
            default:
                if(ptm->tm_mday > 30)
                {
                    ptm->tm_mday-=30;
                    ptm->tm_mon+=1;
                }
                break;
        }
        if(ptm->tm_mon > 12)
        {
            ptm->tm_mon-=12;
            ptm->tm_year+=1;
        }
    }
    return ptm;
}

void printf_with_time(char *dat);
void free_celloc_memory(char ***addr, int num);
void line_feed(char *line);
void set_value(char *file, char *numb);
void set_array_value(char **file, int num, char *value);
void charge_value(char *i);
void check_read_file(char *file);
void read_option(uint *last_modify_time, uchar num, uchar is_temp_wall);
void step_charge_ctl(char *value);
void powel_ctl(void);
int list_dir(char *path, char ***ppp);
