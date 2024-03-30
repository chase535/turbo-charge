#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "some_ctrl.h"
#include "read_option.h"
#include "value_set.h"
#include "my_malloc.h"
#include "foreground_app.h"
#include "printf_with_time.h"

//阶梯充电的控制，1为开启0为关闭
void step_charge_ctl(char *value)
{
    check_read_file("/sys/class/power_supply/battery/step_charging_enabled");
    set_value("/sys/class/power_supply/battery/step_charging_enabled", value);
    set_value("/sys/class/power_supply/battery/sw_jeita_enabled", value);
}

//控制能否进行充电，1为充电0为暂停充电
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

//电量控制
void powel_ctl(int *last_charge_stop, int *charge_is_stop)
{
    int charge_stop=read_one_option("CHARGE_STOP"),power_int=0;
    char power[10];
    if(*last_charge_stop == -1) *last_charge_stop=charge_stop;
    if(read_one_option("POWER_CTRL") == 1)
    {
        read_file("/sys/class/power_supply/battery/capacity", power, sizeof(power));
        power_int=atoi(power);
        if(*last_charge_stop != charge_stop && *charge_is_stop)
        {
            if(power_int < charge_stop)
            {
                printf_with_time("手机当前电量为%d%%，小于新的电量阈值，恢复充电", power_int);
                *charge_is_stop=0;
                charge_ctl("1");
            }
            else
            {
                printf_with_time("手机当前电量为%d%%，大于等于新的电量阈值，保持停止充电状态", power_int);
                charge_ctl("0");
            }
            *last_charge_stop=charge_stop;
        }
        if(power_int >= charge_stop && !(*charge_is_stop))
        {
            printf_with_time("当前电量为%d%%，大于等于停止充电的电量阈值，停止充电", power_int);
            *charge_is_stop=1;
            charge_ctl("0");
        }
        if(power_int <= read_one_option("CHARGE_START") && *charge_is_stop)
        {
            printf_with_time("当前电量为%d%%，小于等于恢复充电的电量阈值，恢复充电", power_int);
            *charge_is_stop=0;
            charge_ctl("1");
        }
    }
    else
    {
        if(*charge_is_stop)
        {
            printf_with_time("电量控制关闭，恢复充电");
            *charge_is_stop=0;
        }
        if(*last_charge_stop != charge_stop) *last_charge_stop=charge_stop;
        charge_ctl("1");
    }
}

/*
“伪”旁路充电控制
thread1为用来存储进程信息的pthread_t结构体
android_version为安卓版本
last_appname为上一次获取到的前台应用包名
is_bypass为上一次获取到的前台应用是否位于配置列表中，与主函数进行通信
screen_is_off为手机是否处于锁屏状态，与主函数进行通信
current_max_file为存储电流文件的二级指针变量，与current_max_file_num配套使用
current_max_file_num为电流文件的个数
*/
void bypass_charge_ctl(pthread_t *thread1, int *android_version, char *last_appname, int *is_bypass, int *screen_is_off, char **current_max_file, int current_max_file_num)
{
    char name[APP_PACKAGE_NAME_MAX_SIZE]={0};
    uchar in_list=0;
    FILE *fp;
    static char **bypass_app_package_name=NULL;
    static uint bypass_file_last_modify_time=0,bypass_app_num=0;
    struct stat statbuf;
    /*
    为了不使获取前台应用包名拖累主程序的执行效率，所以使用了子线程方案
    如果配置文件的BYPASS_CHARGE值为1且ForegroundAppName值为空(没有子线程正在执行)，则创建子线程
    此子线程用来获取前台应用包名
    */
    pthread_mutex_lock((pthread_mutex_t *)&mutex_foreground_app);
    if(read_one_option("BYPASS_CHARGE") == 1 && !strlen((char *)ForegroundAppName))
    {
        strncpy((char *)ForegroundAppName, "chase535", APP_PACKAGE_NAME_MAX_SIZE-1);
        pthread_create(thread1, NULL, get_foreground_appname, (void *)android_version);
        pthread_detach(*thread1);
    }
    //如果ForegroundAppName值不是初始值，则代表获取到了前台应用包名，则进行匹配并作出相应行动
    else if(read_one_option("BYPASS_CHARGE") == 1 && strlen((char *)ForegroundAppName) && strcmp((char *)ForegroundAppName, "chase535"))
    {
        //如果手机从锁屏状态恢复且在锁屏前处于“伪”旁路供电模式，则恢复此模式
        if(strcmp((char *)ForegroundAppName, "screen_is_off"))
        {
            if(*screen_is_off)
            {
                if(*is_bypass) printf_with_time("手机屏幕开启，恢复“伪”旁路供电模式");
                *screen_is_off=0;
            }
            //读取“伪”旁路供电的配置文件
            check_read_file(bypass_charge_file);
            stat(option_file, &statbuf);
            if(statbuf.st_mtime != bypass_file_last_modify_time)
            {
                bypass_app_num=0;
                if(bypass_app_package_name != NULL) free_malloc_memory(&bypass_app_package_name, bypass_app_num);
                bypass_app_package_name=(char **)my_calloc(1, sizeof(char *));
                fp=fopen(bypass_charge_file, "rt");
                while(fgets(name, sizeof(name), fp) != NULL)
                {
                    line_feed(name);
                    //跳过以英文井号开头的行及空行
                    if(!strlen(name) || (strstr(name, "#") != NULL && !strstr(name, "#"))) continue;
                    bypass_app_num++;
                    bypass_app_package_name=(char **)my_realloc(bypass_app_package_name, sizeof(char *)*bypass_app_num);
                    bypass_app_package_name[bypass_app_num-1]=(char *)my_calloc(1, sizeof(char)*APP_PACKAGE_NAME_MAX_SIZE);
                    strncpy(bypass_app_package_name[bypass_app_num-1], name, APP_PACKAGE_NAME_MAX_SIZE-1);
                }
                fclose(fp);
                fp=NULL;
            }
            //判断前台应用包名是否在配置文件中
            for(uint i=0;i < bypass_app_num; i++)
            {
                if(!strcmp((char *)ForegroundAppName, bypass_app_package_name[i]))
                {
                    in_list=1;
                    break;
                }
            }
            //如果前台应用包名在配置文件中
            if(in_list)
            {
                //如果之前不在“伪”旁路供电模式而切换应用后在了，就打印相关信息
                if(!(*is_bypass))
                {
                    printf_with_time("当前前台应用为%s，位于“伪”旁路供电配置列表中，进入“伪”旁路供电模式", ForegroundAppName);
                    *is_bypass=1;
                }
                //如果之前在“伪”旁路供电模式而切换应用后还在，就打印相关信息
                else if(strcmp(last_appname, (char *)ForegroundAppName)) printf_with_time("前台应用切换为%s，位于“伪”旁路供电配置列表中，保持“伪”旁路供电模式", ForegroundAppName);
                //限制电流，“伪”旁路供电
                set_array_value(current_max_file, current_max_file_num, BYPASS_CHARGE_CURRENT);
            }
            else
            {
                //如果之前在“伪”旁路供电模式而切换应用后不在，就打印相关信息
                if(*is_bypass)
                {
                    //分为两种情况，一种为未切换前台应用但应用包名从“伪”旁路供电配置列表中移除，一种为切换到了“伪”旁路供电配置列表中没有的应用
                    if(strcmp(last_appname, (char *)ForegroundAppName)) printf_with_time("前台应用切换为%s，不在“伪”旁路供电配置列表中，恢复正常充电模式", ForegroundAppName);
                    else printf_with_time("%s已从“伪”旁路供电配置列表中移除，恢复正常充电模式", ForegroundAppName);
                    *is_bypass=0;
                }
            }
            strncpy(last_appname, (char *)ForegroundAppName, APP_PACKAGE_NAME_MAX_SIZE-1);
        }
        else
        {
            //如果手机进入锁屏状态且在锁屏前处于“伪”旁路供电模式，则暂时恢复正常充电模式
            if(!(*screen_is_off))
            {
                if(*is_bypass) printf_with_time("手机屏幕关闭，暂时进入正常充电模式");
                *screen_is_off=1;
            }
        }
    }
    //子线程结束运行或者获取不到前台应用包名，则清空上一次获取到的前台应用包名并恢复正常充电模式
    else
    {
        if(strlen(last_appname)) memset(last_appname, 0, APP_PACKAGE_NAME_MAX_SIZE*sizeof(char));
        if(*is_bypass) *is_bypass=0;
    }
    pthread_mutex_unlock((pthread_mutex_t *)&mutex_foreground_app);
}
