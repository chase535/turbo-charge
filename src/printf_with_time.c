#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include "printf_with_time.h"

//以printf的标准对函数的参数进行检查、格式化，并在字符串前面加上时间
void printf_with_time(const char *format, ...) __attribute__((__format__(__printf__, 1, 2)));
void printf_with_time(const char *format, ...)
{
    char buffer[1024],time_char[30];
    time_t cur_time;
    struct tm *time_now;
    va_list ap;
    //使用vsnprintf函数拼接格式化字符串和可变参数
    va_start(ap, format);
    vsnprintf(buffer, 1024, format, ap);
    va_end(ap);
    time(&cur_time);
    time_now=localtime(&cur_time);
    strftime(time_char, 30, "%Y.%m.%d %H:%M:%S UTC%z", time_now);
    //在拼接后的字符串前面加上时间
    printf("[ %s ] %s\n", time_char, buffer);
    fflush(stdout);
}
