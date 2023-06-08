#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "sys/wait.h"
#include "sys/types.h"

#include "printf_with_time.h"
#include "foreground_app.h"

/*
获取前台应用的包名以配合“伪”旁路供电功能使用
我是干嵌入式系统开发的，对AndroidNDK不熟悉，无法使用纯C语言实现，所以只能使用popen管道执行Shell命令来获取
但这就导致了执行效率的大幅下降，我无能为力，只能等待其他大佬的PR
*/
void *get_foreground_appname(void *android_version)
{
    char result[200],screen[50],*tmpchar1=NULL,*tmpchar2=NULL;
    pid_t status=0;
    FILE *fp=NULL;
    while(bypass_charge == 1)
    {
        //判断是否为锁屏状态，如果是则无法获取应用包名，直接将全局变量ForegroundAppName赋值为screen_is_off
        fp=popen("dumpsys deviceidle | grep 'mScreenOn'", "r");
        if(fp == NULL) printf_with_time("无法创建管道通信，跳过本次循环！");
        fgets(screen, sizeof(screen), fp);
        line_feed(screen);
        status=pclose(fp);
        if(status == -1) goto close_pipe_err;
        else if(!WIFEXITED(status))
        {
            printf_with_time("获取屏幕是否开启的Shell命令执行出错，跳过本次循环！");
            fp=NULL;
            sleep(5);
            //添加进程取消点，否则无法通过调用pthread_exit()函数取消进程
            //下面所有pthread_testcancel()函数的作用都同上
            pthread_testcancel();
            continue;
        }
        fp=NULL;
        //此Shell命令的返回值格式为  mScreenOn=true
        //若为true则没有锁屏，若为false则处于锁屏状态
        if(strcmp(strstr(screen, "=")+1, "true"))
        {
            strcpy((char *)ForegroundAppName, "screen_is_off");
            sleep(5);
            pthread_testcancel();
            continue;
        }
        /*
        如果非锁屏状态，则获取前台应用包名
        安卓10及以上可使用dumpsys activity lru获取应用状态
        安卓10以下就只能用dumpsys activity o获取，执行效率慢
        然后再使用grep提取出包含关键字的行，此行即为前台应用的详情
        */
        fp=(*((int *)android_version) < 10)?popen("dumpsys activity o | grep ' (top-activity)'", "r"):popen("dumpsys activity lru | grep ' TOP'", "r");
        if(fp == NULL) printf_with_time("无法创建管道通信，跳过本次循环！");
        fgets(result, sizeof(result), fp);
        line_feed(result);
        status=pclose(fp);
        if(status == -1)
        {
            close_pipe_err:
            printf_with_time("关闭管道通信时出错，跳过本次循环！");
            fp=NULL;
            sleep(5);
            pthread_testcancel();
            continue;
        }
        else if(!WIFEXITED(status))
        {
            printf_with_time("获取前台应用包名的Shell命令执行出错，跳过本次循环！");
            fp=NULL;
            sleep(5);
            pthread_testcancel();
            continue;
        }
        fp=NULL;
        tmpchar1=(*((int *)android_version) < 10)?strstr(result, "/TOP"):strstr(result, " TOP");
        if(tmpchar1 == NULL)
        {
            can_not_get:
            printf_with_time("无法获取前台应用包名，跳过本次循环！");
            sleep(5);
            pthread_testcancel();
            continue;
        }
        /*
        从上一步获取到的前台应用的详情中截取应用包名
        dumpsys activity lru获取到的详情格式为  #98: fg     TOP  LCMN 4493:com.termux/u0a351 act:activities|recents
        dumpsys activity o获取到的详情格式为  Proc # 0: fg     T/A/TOP  LCMN  t: 0 4493:com.termux/u0a351 (top-activity)
        */
        tmpchar2=(*((int *)android_version) < 10)?strstr(strstr(tmpchar1, ":")+1, ":"):strstr(tmpchar1, ":");
        if(tmpchar2 == NULL) goto can_not_get;
        tmpchar1=strstr(tmpchar2, "/");
        if(tmpchar1 == NULL) goto can_not_get;
        *tmpchar1='\0';
        pthread_testcancel();
        //将前台应用包名赋值给ForegroundAppName
        strncpy((char *)ForegroundAppName, tmpchar2+1, sizeof(ForegroundAppName)/sizeof(char)-1);
        tmpchar1=NULL;
        tmpchar2=NULL;
        sleep(5);
        pthread_testcancel();
    }
    //如果配置文件的BYPASS_CHARGE值不为1，则退出循环并将ForegroundAppName清空
    memset((void *)ForegroundAppName, 0, sizeof(ForegroundAppName));
    return NULL;
}

//获取安卓版本，因为只有安卓7及以上才能使用Shell命令获取安卓版本
int check_android_version()
{
    char android_version_char[5];
    int android_version=0;
    pid_t status=0;
    FILE *fp=NULL;
    fp=popen("getprop ro.build.version.release", "r");
    if(fp == NULL)
    {
        printf_with_time("无法创建管道通信，故无法获取安卓版本，“伪”旁路供电功能失效！");
        return 0;
    }
    fgets(android_version_char, sizeof(android_version_char), fp);
    line_feed(android_version_char);
    status=pclose(fp);
    if(status == -1 || !WIFEXITED(status) || !strlen(android_version_char))
    {
        printf_with_time("无法获取安卓版本，而只有安卓7及以上能够通过Shell命令获取前台应用，所以必须要获取安卓版本进行判断，“伪”旁路供电功能失效！");
        fp=NULL;
        return 0;
    }
    fp=NULL;
    android_version=atoi(android_version_char);
    if(android_version < 7)
    {
        snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "安卓7及以下无法通过Shell命令获取前台应用，当前版本为安卓%d，“伪”旁路供电功能失效！", android_version);
        printf_with_time(chartmp);
        return 0;
    }
    return android_version;
}