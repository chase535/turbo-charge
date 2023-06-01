#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"

#include "printf_with_time.h"
#include "foreground_app.h"

void *get_foreground_appname()
{
    char result[200],*tmpchar1=NULL,*tmpchar2=NULL;
    FILE *fp=NULL;
    while(bypass_charge == 1)
    {
        fp=popen("/system/bin/dumpsys activity lru | grep ' TOP'","r");
        if(fp == NULL)
        {
            can_not_get:
            printf_with_time("无法获取前台应用包名！");
            sleep(1);
            continue;
        }
        fread(result,1,sizeof(result),fp);
        tmpchar1=strstr(result," TOP");
        if(tmpchar1 == NULL) goto can_not_get;
        tmpchar2=strstr(tmpchar1,":");
        if(tmpchar2 == NULL) goto can_not_get;
        tmpchar1=strstr(tmpchar2,"/");
        if(tmpchar1 == NULL) goto can_not_get;
        memcpy((void *)ForegroundAppName,tmpchar2+1,(tmpchar1-tmpchar2)-2);
        tmpchar1=NULL;
        tmpchar2=NULL;
        printf("%s",ForegroundAppName);
        sleep(1);
    }
    memset((void *)ForegroundAppName,0,sizeof(ForegroundAppName));
    pthread_exit(NULL);
    return NULL;
}
