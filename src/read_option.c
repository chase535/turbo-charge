#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "unistd.h"
#include "dirent.h"

#include "read_option.h"

void read_option(uint *last_modify_time, uchar num, uchar is_temp_wall)
{
    FILE *fc;
    char option_tmp[42], option[100], *value;
    uchar opt,value_stat[OPTION_QUANTITY]={0},i;
    struct stat statbuf;
    check_read_file(option_file);
    stat(option_file, &statbuf);
    if(statbuf.st_mtime == *last_modify_time && num) return;
    *last_modify_time=statbuf.st_mtime;
    fc=fopen(option_file, "rt");
    while(fgets(option, sizeof(option), fc) != NULL)
    {
        line_feed(option);
        if((strstr(option, "#") != NULL && strstr(option, "#") == 0) || strlen(option) == 0) continue;
        for(opt=0;opt < OPTION_QUANTITY;opt++)
        {
            snprintf(option_tmp, 42, "%s=", options[opt]);
            if(strstr(option, option_tmp) == NULL) continue;
            value_stat[opt]=10;
            if(strcmp(option, option_tmp) == 0) value_stat[opt]=1;
            else
            {
                value=option+strlen(option_tmp);
                for(i=0;i < strlen(value);i++)
                {
                    if((int)value[i] < 48 || (int)value[i] > 57)
                    {
                        value_stat[opt]=2;
                        break;
                    }
                }
            }
            if(value_stat[opt] == 10 && atoi(value) < 0) value_stat[opt]=3;
            if(value_stat[opt] == 10) opt_new[opt]=atoi(value);
        }
    }
    fclose(fc);
    fc=NULL;
    if(num)
    {
        for(opt=0;opt < OPTION_QUANTITY;opt++)
        {
            if(value_stat[opt] == 1) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "%s的值为空，故程序沿用上一次的值%d", options[opt], opt_new[opt]);
            else if(value_stat[opt] == 2) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "%s的值不是由纯数字组成，故程序沿用上一次的值%d", options[opt], opt_new[opt]);
            else if(value_stat[opt] == 3) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "%s的值小于0，这是不被允许的，故程序沿用上一次的值%d", options[opt], opt_new[opt]);
            else if(value_stat[opt] == 0) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "%s不存在，故程序沿用上一次的值%d", options[opt], opt_new[opt]);
            if(value_stat[opt] != 10) printf_with_time(chartmp);
            if(opt_old[opt] != opt_new[opt])
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "%s值发生改变，新%s值为%d", options[opt], options[opt], opt_new[opt]);
                printf_with_time(chartmp);
                if(opt == 5 && opt_old[opt] < opt_new[opt]) tmp[4]=1;
                if(is_temp_wall == 1 && (opt == 7 && opt_old[opt] < opt_new[opt])) tmp[3]=1;
                opt_old[opt]=opt_new[opt];
            }
        }
    }
    else
    {
        for(opt=0;opt < OPTION_QUANTITY;opt++)
        {
            if(value_stat[opt] == 1) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s的值为空，故程序使用默认值%d", options[opt], opt_new[opt]);
            else if(value_stat[opt] == 2) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s的值不是由纯数字组成，故程序使用默认值%d", options[opt], opt_new[opt]);
            else if(value_stat[opt] == 3) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s的值小于0，这是不被允许的，故程序使用默认值%d", options[opt], opt_new[opt]);
            else if(value_stat[opt] == 0) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s不存在，故程序使用默认值%d", options[opt], opt_new[opt]);
            if(value_stat[opt] != 10) printf_with_time(chartmp);
            opt_old[opt]=opt_new[opt];
        }
    }
}
