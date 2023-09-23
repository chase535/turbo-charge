#ifndef _FOREGROUND_APP_H
#define _FOREGROUND_APP_H

#include "main.h"

extern volatile char ForegroundAppName[100];

void *get_foreground_appname(void *android_version);
int check_android_version();

#endif
