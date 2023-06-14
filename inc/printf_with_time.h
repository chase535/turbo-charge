#ifndef _PRINTF_WITH_TIME_H
#define _PRINTF_WITH_TIME_H

#include "main.h"

extern struct tm time_utc8_now;

void get_utc8_time(struct tm *ptm);

//特别注意，因为是用的define而不是定义函数，所以使用时不得将if、for等语句进行简写，也不得在二目运算符中使用，必须写出带大括号的完整语句块
#define printf_with_time(FORMAT,...)\
{\
    get_utc8_time(&time_utc8_now);\
    printf("[ %04d.%02d.%02d %02d:%02d:%02d UTC+8 ] "FORMAT"\n", time_utc8_now.tm_year, time_utc8_now.tm_mon, time_utc8_now.tm_mday, time_utc8_now.tm_hour, time_utc8_now.tm_min, time_utc8_now.tm_sec, ##__VA_ARGS__);\
    fflush(stdout);\
}

#endif
