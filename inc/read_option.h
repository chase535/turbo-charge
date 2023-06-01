#ifndef _READ_OPTION_H
#define _READ_OPTION_H

#include "main.h"

void read_option(uint *last_modify_time, uchar num, uchar tmp[], uchar is_temp_wall, int *cycle_time,
                uchar *option_force_temp, char current_max_char[20], int *step_charging_disabled, int *temp_ctrl,
                int *step_charging_disabled_threshold, int *temp_max, char highest_temp_current_char[20], int *recharge_temp);

#endif
