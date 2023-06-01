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
        tmpchar1=NULL;
        tmpchar2=NULL;
        fp=popen("/system/bin/dumpsys activity lru | grep ' TOP'","r");
        if(fp == NULL) goto can_not_get_fp_null;
        fgets(result,sizeof(result),fp);
        tmpchar1=strstr(result," TOP");
        if(tmpchar1 == NULL)
        {
            can_not_get:
            pclose(fp);
            fp=NULL;
            can_not_get_fp_null:
            printf_with_time("无法获取前台应用包名！");
            sleep(1);
            continue;
        }
        tmpchar2=strstr(tmpchar1,":");
        if(tmpchar2 == NULL) goto can_not_get;
        tmpchar1=strstr(tmpchar2,"/");
        if(tmpchar1 == NULL) goto can_not_get;
        *tmpchar1='\0';
        strncpy((char *)ForegroundAppName,tmpchar2+1,sizeof(ForegroundAppName)/sizeof(char)-1);
        pclose(fp);
        fp=NULL;
        printf("%s\n",ForegroundAppName);
        sleep(1);
    }
    memset((void *)ForegroundAppName,0,sizeof(ForegroundAppName));
    pthread_exit(NULL);
    return NULL;
}
