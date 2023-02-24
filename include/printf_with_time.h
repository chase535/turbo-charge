#ifndef _PRINTF_WITH_TIME_H
#define _PRINTF_WITH_TIME_H

#include "stdio.h"
#include "time.h"

static struct tm *get_utc8_time(void)
{
    time_t cur_time;
    struct tm *ptm;
    time(&cur_time);
    ptm=gmtime(&cur_time);
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
    return ptm;
}

void printf_with_time(char *dat);

#endif
