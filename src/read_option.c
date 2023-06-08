#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "unistd.h"
#include "dirent.h"

#include "read_option.h"
#include "printf_with_time.h"

/*
读取配置文件并赋值给程序内部变量
last_modify_time为上次读取配置文件时配置文件的文件修改时间
num为是否第一次读取配置，0为是1为否
tmp为数组，用来与主函数进行通信
is_temp_wall为是否碰到了温度墙，此值将会影响函数向tmp数组进行赋值
*/
void read_options(uint *last_modify_time, uchar num, uchar tmp[], uchar is_temp_wall)
{
    FILE *fc;
    char option_tmp[42], option[100], *value;
    int new_value=0;
    uchar opt,value_stat[OPTION_QUANTITY]={0},i=0;
    struct stat statbuf;
    check_read_file(option_file);
    //先获取文件修改时间，若本次的文件修改时间与上次相等，证明配置文件未修改，跳过读取
    stat(option_file, &statbuf);
    if(statbuf.st_mtime == *last_modify_time && num) return;
    *last_modify_time=statbuf.st_mtime;
    fc=fopen(option_file, "rt");
    while(fgets(option, sizeof(option), fc) != NULL)
    {
        line_feed(option);
        //跳过以英文井号开头的行及空行
        if(!strlen(option) || (strstr(option, "#") != NULL && !strstr(option, "#"))) continue;
        for(opt=0;opt < OPTION_QUANTITY;opt++)
        {
            //将配置名与等号就行拼接，用来进行匹配
            snprintf(option_tmp, 42, "%s=", options[opt].name);
            if(strstr(option, option_tmp) == NULL) continue;
            //value_stat数组存储配置文件中每个变量值的状态，详情直接看后面判断value_stat数组的值的相关代码
            value_stat[opt]=10;
            //判断变量值是否合法，合法则对程序内部变量进行赋值，否则将状态存入value_stat数组中
            if(!strcmp(option, option_tmp)) value_stat[opt]=1;
            else
            {
                value=option+strlen(option_tmp);
                for(i=0;i < strlen(value);i++)
                {
                    if(((int)value[i] < 48 || (int)value[i] > 57) && ((!i && ((int)value[i] != 45 || strlen(value) == 1)) || i))
                    {
                        value_stat[opt]=2;
                        break;
                    }
                }
            }
            if(value_stat[opt] != 10) continue;
            new_value=atoi(value);
            if(new_value < 0) value_stat[opt]=3;
            else if(!strcmp(options[opt].name, "CYCLE_TIME") && !new_value) value_stat[opt]=4;
            if(value_stat[opt] == 10 && options[opt].value != new_value)
            {
                if(num)
                {
                    if(!strcmp(options[opt].name, "CHARGE_STOP") && options[opt].value < new_value) tmp[4]=1;
                    if(!strcmp(options[opt].name, "TEMP_MAX") && is_temp_wall == 1 && options[opt].value < new_value) tmp[3]=1;
                }
                options[opt].value=new_value;
                value_stat[opt]=100;
            }
        }
    }
    fclose(fc);
    fc=NULL;
    for(opt=0;opt < OPTION_QUANTITY;opt++)
    {
        //通过判断是否第一次运行来进行不同的字符串输出
        if(num)
        {
            if(value_stat[opt] == 0) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s不存在，故程序沿用上一次的值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 1) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新%s的值为空，故程序沿用上一次的值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 2) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新%s的值不是由纯数字组成，故程序沿用上一次的值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 3) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新%s的值小于0，这是不被允许的，故程序沿用上一次的值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 4) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新%s的值为0，这是不被允许的，故程序沿用上一次的值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 100) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "%s的值更改为%d", options[opt].name, options[opt].value);
            if(value_stat[opt] != 10) printf_with_time(chartmp);
        }
        else
        {
            if(value_stat[opt] == 0) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s不存在，故程序使用默认值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 1) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s的值为空，故程序使用默认值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 2) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s的值不是由纯数字组成，故程序使用默认值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 3) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s的值小于0，这是不被允许的，故程序使用默认值%d", options[opt].name, options[opt].value);
            else if(value_stat[opt] == 4) snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "配置文件中%s的值为0，这是不被允许的，故程序使用默认值%d", options[opt].name, options[opt].value);
            if(!(value_stat[opt] == 10 || value_stat[opt] == 100)) printf_with_time(chartmp);
        }
    }
}

//读取单个配置的值
int read_one_option(char *name)
{
    int i=0,value=-1;
    for(i=0;i < OPTION_QUANTITY;i++)
    {
        if(!strcmp(options[i].name, name))
        {
            value=options[i].value;
            break;
        }
    }
    if(value < 0)
    {
        printf_with_time("无法获取变量，程序发生了内部错误，请立即前往Github进行反馈！");
        exit(98765);
    }
    return value;
}
