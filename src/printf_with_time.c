#include "stdio.h"
#include "time.h"
#include "string.h"
#include "stdarg.h"

#include "printf_with_time.h"

//获取本地时间并将时间转换为北京时间(UTC+8)
void get_utc8_time(struct tm *ptm)
{
    time_t cur_time;
    time(&cur_time);
    /*
    新手特别容易犯的一个错误，形参的指针变量不能通过改变指向的地址来间接改变所存数据
    在下一行代码中的体现就是不能通过 ptm=gmtime(&cur_time) 来直接将形参的指针变量重新指向另一块地址
    只能通过memcpy等函数将其余地址所存数据复制到原形参指向的地址中，也就是只能直接改变原地址的数据
    */
    memcpy(ptm, gmtime(&cur_time), sizeof(struct tm));
    //手动将协调世界时转换为北京时间
    ptm->tm_year+=1900;
    ptm->tm_mon+=1;
    ptm->tm_hour+=8;
    if(ptm->tm_hour > 23)
    {
        ptm->tm_hour-=24;
        ptm->tm_mday+=1;
        switch(ptm->tm_mon)
        {
            case 1: case 3: case 5: case 7: case 8: case 10: case 12:
                if(ptm->tm_mday > 31)
                {
                    ptm->tm_mday-=31;
                    ptm->tm_mon+=1;
                }
                break;
            case 2:
                if(((ptm->tm_year%4 == 0) && (ptm->tm_year%100 != 0)) || (ptm->tm_year%400 == 0))
                {
                    if(ptm->tm_mday > 29)
                    {
                        ptm->tm_mday-=29;
                        ptm->tm_mon+=1;
                    }
                }
                else
                {
                    if(ptm->tm_mday > 28)
                    {
                        ptm->tm_mday-=28;
                        ptm->tm_mon+=1;
                    }
                }
                break;
            default:
                if(ptm->tm_mday > 30)
                {
                    ptm->tm_mday-=30;
                    ptm->tm_mon+=1;
                }
                break;
        }
        if(ptm->tm_mon > 12)
        {
            ptm->tm_mon-=12;
            ptm->tm_year+=1;
        }
    }
}

//以printf的标准对函数的参数进行检查、格式化，并在字符串前面加上时间
void printf_with_time(const char *format, ...) __attribute__((__format__(__printf__, 1, 2)));
void printf_with_time(const char *format, ...)
{
    char buffer[1024];
    struct tm time_utc8_now;
    va_list ap;
    //使用vsnprintf函数拼接格式化字符串和可变参数
    va_start(ap, format);
    vsnprintf(buffer, 1024, format, ap);
    va_end(ap);
    get_utc8_time(&time_utc8_now);
    //在拼接后的字符串前面加上时间
    printf("[ %04d.%02d.%02dT%02d:%02d:%02d UTC+8 ] %s\n", time_utc8_now.tm_year, time_utc8_now.tm_mon, time_utc8_now.tm_mday, time_utc8_now.tm_hour, time_utc8_now.tm_min, time_utc8_now.tm_sec, buffer);
    fflush(stdout);
}
