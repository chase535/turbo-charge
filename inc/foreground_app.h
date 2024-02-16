#ifndef _FOREGROUND_APP_H
#define _FOREGROUND_APP_H

#include "main.h"

extern volatile char ForegroundAppName[APP_PACKAGE_NAME_MAX_SIZE];

void *get_foreground_appname(void *android_version);
int check_android_version();

#endif
