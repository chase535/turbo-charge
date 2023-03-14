#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"

#include "global_function.h"
#include "global_variable.h"

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
