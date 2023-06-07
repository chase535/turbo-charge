#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "pthread.h"

#include "some_ctrl.h"
#include "value_set.h"
#include "foreground_app.h"
#include "printf_with_time.h"

void step_charge_ctl(char *value)
{
    check_read_file("/sys/class/power_supply/battery/step_charging_enabled");
    set_value("/sys/class/power_supply/battery/step_charging_enabled", value);
    set_value("/sys/class/power_supply/battery/sw_jeita_enabled", value);
}

void charge_ctl(char *i)
{
    set_value("/sys/class/power_supply/battery/charging_enabled", i);
    set_value("/sys/class/power_supply/battery/battery_charging_enabled", i);
    if(atoi(i))
    {
        set_value("/sys/class/power_supply/battery/input_suspend", "0");
        set_value("/sys/class/qcom-battery/restricted_charging", "0");
    }
    else
    {
        set_value("/sys/class/power_supply/battery/input_suspend", "1");
        set_value("/sys/class/qcom-battery/restricted_charging", "1");
    }
}

void powel_ctl(uchar tmp[])
{
    int power_control=0,charge_stop=0,charge_start=0,i=0;
    for(i=0;i < OPTION_QUANTITY;i++)
    {
        if(!strcmp(options[i].name, "POWER_CTRL")) power_control=options[i].value;
        else if(!strcmp(options[i].name, "CHARGE_STOP")) charge_stop=options[i].value;
        else if(!strcmp(options[i].name, "CHARGE_START")) charge_start=options[i].value;
    }
    if(power_control == 1)
    {
        FILE *fd;
        struct stat statbuf;
        check_read_file("/sys/class/power_supply/battery/capacity");
        stat("/sys/class/power_supply/battery/capacity", &statbuf);
        char power[statbuf.st_size+1];
        fd=fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fd);
        fclose(fd);
        fd=NULL;
        line_feed(power);
        if(tmp[2] && tmp[4])
        {
            if(atoi(power) < charge_stop)
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新的停止充电的电量阈值高于旧的电量阈值，且手机当前电量为%s%%，小于新的电量阈值，恢复充电", power);
                printf_with_time(chartmp);
                charge_ctl("1");
                tmp[2]=0;
            }
            else
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新的停止充电的电量阈值高于旧的电量阈值，但手机当前电量为%s%%，大于等于新的电量阈值，停止充电", power);
                printf_with_time(chartmp);
                charge_ctl("0");
                tmp[2]=1;
            }
            tmp[4]=0;
        }
        if(atoi(power) >= charge_stop)
        {
            if(!tmp[2])
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "当前电量为%s%%，大于等于停止充电的电量阈值，停止充电", power);
                printf_with_time(chartmp);
                tmp[2]=1;
            }
            charge_ctl("0");
        }
        if(atoi(power) <= charge_start)
        {
            if(tmp[2])
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "当前电量为%s%%，小于等于恢复充电的电量阈值，恢复充电", power);
                printf_with_time(chartmp);
                tmp[2]=0;
            }
            charge_ctl("1");
        }
    }
    else
    {
        if(tmp[2])
        {
            printf_with_time("电量控制关闭，恢复充电");
            tmp[2]=0;
        }
        charge_ctl("1");
    }
}

void bypass_charge_ctl(pthread_t *thread1, int *android_version, char last_appname[100], int *is_bypass, char **current_max_file, int current_max_file_num)
{
    char name[100];
    uchar in_list=0;
    FILE *fp;
    if(bypass_charge == 1 && !strlen((char *)ForegroundAppName))
    {
        strcpy((char *)ForegroundAppName, "chase535");
        pthread_create(thread1, NULL, get_foreground_appname, (void *)android_version);
        pthread_detach(*thread1);
    }
    else if(bypass_charge == 1 && strlen((char *)ForegroundAppName) && !strcmp((char *)ForegroundAppName, "chase535"))
    {
        check_read_file(bypass_charge_file);
        fp=fopen(bypass_charge_file, "rt");
        while(fgets(name, sizeof(name), fp) != NULL)
        {
            line_feed(name);
            if(!strlen(name) || (strstr(name, "#") != NULL && !strstr(name, "#"))) continue;
            if(!strcmp((char *)ForegroundAppName, name)) in_list=1;
        }
        fclose(fp);
        fp=NULL;
        if(in_list)
        {
            if(!(*is_bypass))
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "当前前台应用为%s，位于旁路供电配置列表中，开始“伪”旁路供电", ForegroundAppName);
                printf_with_time(chartmp);
            }
            else
            {
                if(strcmp(last_appname, (char *)ForegroundAppName))
                {
                    snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "前台应用切换为%s，位于旁路供电配置列表中，保持“伪”旁路供电", ForegroundAppName);
                    printf_with_time(chartmp);
                }
            }
            *is_bypass=1;
            set_array_value(current_max_file, current_max_file_num, BYPASS_CHARGE_CURRENT);
        }
        else
        {
            if(*is_bypass)
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "前台应用切换为%s，不在旁路供电配置列表中，恢复正常充电", ForegroundAppName);
                printf_with_time(chartmp);
                *is_bypass=0;
            }
        }
        strcpy(last_appname, (char *)ForegroundAppName);
    }
    else if(strlen(last_appname)) memset(last_appname, 0, 100*sizeof(char));
}
