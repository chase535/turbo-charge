#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "errno.h"

#include "printf_with_time.h"
#include "foreground_app.h"

int check_android_version()
{
    char android_version_char[5];
    int android_version=0;
    FILE *fp=NULL;
    fp=popen("getprop ro.build.version.release", "r");
    if(fp == NULL)
    {
        printf_with_time("无法创建管道通信，不能执行Shell命令，故无法获取前台应用包名，“伪”旁路供电功能失效！");
        return 0;
    }
    fgets(android_version_char, sizeof(android_version_char), fp);
    if(pclose(fp) || !strlen(android_version_char))
    {
        printf_with_time("无法获取安卓版本，而安卓7-9、10+获取前台应用的命令不同，故无法获取前台应用包名，“伪”旁路供电功能失效！");
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

void *get_foreground_appname(void *android_version)
{
    char result[200],*tmpchar1=NULL,*tmpchar2=NULL;
    FILE *fp=NULL;
    while(bypass_charge == 1)
    {
        fp=(*((int *)android_version) < 10)?popen("dumpsys activity o | grep ' (top-activity)'", "r"):popen("dumpsys activity lru | grep ' TOP'", "r");
        if(fp == NULL) printf_with_time("无法创建管道通信！");
        fgets(result, sizeof(result), fp);
        if(pclose(fp))
        {
            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "Shell命令执行出错：%s", strerror(errno));
            printf_with_time(chartmp);
            sleep(5);
            pthread_testcancel();
            continue;
        }
        fp=NULL;
        tmpchar1=strstr(result, " TOP");
        if(tmpchar1 == NULL)
        {
            can_not_get:
            pclose(fp);
            fp=NULL;
            printf_with_time("无法获取前台应用包名！");
            sleep(5);
            pthread_testcancel();
            continue;
        }
        tmpchar2=(*((int *)android_version) < 10)?strstr(strstr(tmpchar1, ":"), ":"):strstr(tmpchar1, ":");
        if(tmpchar2 == NULL) goto can_not_get;
        tmpchar1=strstr(tmpchar2, "/");
        if(tmpchar1 == NULL) goto can_not_get;
        *tmpchar1='\0';
        pthread_testcancel();
        strncpy((char *)ForegroundAppName, tmpchar2+1, sizeof(ForegroundAppName)/sizeof(char)-1);
        tmpchar1=NULL;
        tmpchar2=NULL;
        sleep(5);
        pthread_testcancel();
    }
    memset((void *)ForegroundAppName, 0, sizeof(ForegroundAppName));
    return NULL;
}
