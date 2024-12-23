#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "read_option_file.h"
#include "printf_with_time.h"

/*
读取配置文件并赋值给程序内部变量
*/
void *read_option_file(void *arg)
{
    //文件修改时间需持续存储，所以在循环体外定义
    uint option_last_modify_time=0, sleep_time=*(int *)arg;
    while(1)
    {
        //在循环体内定义变量，这样变量仅存在于单次循环，每次循环结束后变量自动释放，循环开始时变量重新定义
        FILE *fc;
        char option_tmp[42]={0},option[100]={0},*value=NULL;
        uchar opt=0,i=0,option_quantity=1;
        int new_value=0;
        struct stat statbuf;
        ListNode *node=options_head.next,*tmp=options_head.next;
        if(node == NULL || tmp == NULL)
        {
            printf_with_time("存储配置信息的链表中没有节点，程序强制停止运行！");
            exit(9988);
        }
        while((tmp=tmp->next) != NULL) option_quantity++;
        //不定长数组无法在定义时初始化为全0，所以后续会使用memset进行清零
        uchar value_stat[option_quantity];
        check_read_file(option_file);
        //先获取文件修改时间，若本次的文件修改时间与上次相等，证明配置文件未修改，跳过读取
        stat(option_file, &statbuf);
        if(statbuf.st_mtime == option_last_modify_time)
        {
            sleep(sleep_time);
            continue;
        }
        memset(value_stat, 0, sizeof(value_stat));
        pthread_mutex_lock((pthread_mutex_t *)&mutex_options);
        fc=fopen(option_file, "rt");
        while(fgets(option, sizeof(option), fc) != NULL)
        {
            line_feed(option);
            //跳过以英文井号开头的行及空行
            if(!strlen(option) || (strstr(option, "#") != NULL && !strstr(option, "#"))) continue;
            for(opt=0,node=options_head.next;node;opt++,node=node->next)
            {
                //将配置名与等号进行拼接，用来进行匹配
                snprintf(option_tmp, 42, "%s=", node->name);
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
                        if((value[i] < '0' || value[i] > '9') && ((!i && (value[i] != '-' || strlen(value) == 1)) || i))
                        {
                            value_stat[opt]=2;
                            break;
                        }
                    }
                }
                if(value_stat[opt] != 10) continue;
                new_value=atoi(value);
                if(new_value < 0) value_stat[opt]=3;
                else if(!strcmp((char *)(node->name), "CYCLE_TIME") && !new_value) value_stat[opt]=4;
                if(value_stat[opt] == 10 && node->value != new_value)
                {
                    node->value=new_value;
                    value_stat[opt]=100;
                }
            }
        }
        fclose(fc);
        fc=NULL;
        for(opt=0,node=options_head.next;node;opt++,node=node->next)
        {
            //通过判断是否第一次运行来进行不同的字符串输出
            if(option_last_modify_time)
            {
                if(value_stat[opt] == 0) printf_with_time("配置文件中%s不存在，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 1) printf_with_time("新%s的值为空，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 2) printf_with_time("新%s的值不是由纯数字组成，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 3) printf_with_time("新%s的值小于0，这是不被允许的，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 4) printf_with_time("新%s的值为0，这是不被允许的，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 100) printf_with_time("%s的值更改为%d", node->name, node->value);
            }
            else
            {
                if(value_stat[opt] == 0) printf_with_time("配置文件中%s不存在，故程序使用默认值%d", node->name, node->value);
                else if(value_stat[opt] == 1) printf_with_time("配置文件中%s的值为空，故程序使用默认值%d", node->name, node->value);
                else if(value_stat[opt] == 2) printf_with_time("配置文件中%s的值不是由纯数字组成，故程序使用默认值%d", node->name, node->value);
                else if(value_stat[opt] == 3) printf_with_time("配置文件中%s的值小于0，这是不被允许的，故程序使用默认值%d", node->name, node->value);
                else if(value_stat[opt] == 4) printf_with_time("配置文件中%s的值为0，这是不被允许的，故程序使用默认值%d", node->name, node->value);
            }
        }
        option_last_modify_time=statbuf.st_mtime;
        pthread_mutex_unlock((pthread_mutex_t *)&mutex_options);
        sleep(sleep_time);
    }
    return NULL;
}
