#ifndef _PRINTF_WITH_TIME_H
#define _PRINTF_WITH_TIME_H

#include "main.h"

struct tm *get_utc8_time(void);

void printf_with_time(char *dat);

#endif
