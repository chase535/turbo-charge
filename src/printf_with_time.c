#include "printf_with_time.h"

void printf_with_time(char *dat)
{
    struct tm *time_get=get_utc8_time();
    printf("[ %04d.%02d.%02d %02d:%02d:%02d UTC+8 ] %s\r\n", time_get->tm_year, time_get->tm_mon, time_get->tm_mday, time_get->tm_hour, time_get->tm_min, time_get->tm_sec, dat);
    fflush(stdout);
}
