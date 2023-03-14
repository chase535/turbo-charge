#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"

#include "global.h"
#include "printf_with_time.h"

uint opt_old[OPTION_QUANTITY]={0},opt_new[OPTION_QUANTITY]={0,1,0,50000000,15,95,80,52,2000000,45};
uchar tmp[5]={0};
char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
char option_file[]="/data/adb/turbo-charge/option.txt";
char options[OPTION_QUANTITY][40]={"STEP_CHARGING_DISABLED","TEMP_CTRL","POWER_CTRL","CURRENT_MAX",
                                    "STEP_CHARGING_DISABLED_THRESHOLD","CHARGE_STOP","CHARGE_START",
                                    "TEMP_MAX","HIGHEST_TEMP_CURRENT","RECHARGE_TEMP"};

void line_feed(char *line)
{
    char *p;
    p=strchr(line, '\r');
    if(p != NULL) *p='\0';
    p=strchr(line, '\n');
    if(p != NULL) *p='\0';
}

void check_read_file(char *file)
{
    if(file == NULL)
    {
        snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "指针为空，出现异常错误，程序强制退出！");
        printf_with_time(chartmp);
        exit(789);
    }
    if(access(file, F_OK) == 0)
    {
        if(access(file, R_OK) != 0)
        {
            chmod(file, 0644);
            if(access(file, R_OK) != 0)
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "无法读取%s文件，程序强制退出！", file);
                printf_with_time(chartmp);
                exit(1);
            }
        }
    }
    else
    {
        snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "找不到%s文件，程序强制退出！", file);
        printf_with_time(chartmp);
        exit(999);
    }
}
