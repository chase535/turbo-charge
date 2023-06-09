#ifndef _SOME_CTRL_H
#define _SOME_CTRL_H

#include "main.h"

void step_charge_ctl(char *value);
void charge_ctl(char *i);
void powel_ctl(int *last_charge_stop, int *charge_is_stop);
void bypass_charge_ctl(pthread_t *thread1, int *android_version, char last_appname[100], int *is_bypass, int *screen_is_off, char **current_max_file, int current_max_file_num);

#endif
