#ifndef _MAIN_H
#define _MAIN_H

#include "global.h"
#include "options_linkedlist.h"
#include "my_thread.h"

typedef unsigned char uchar;
typedef unsigned int uint;

int list_dir(char *path, char ***ppp);
void line_feed(char *line);
void check_read_file(char *file);
void read_file(char *file_path, char *char_var, int max_char_num);
void check_required_files(uchar *battery_status, uchar *battery_capacity, uchar *power_control,
                          uchar *step_charge, uchar *force_temp, uchar *current_change,
                          int *temp_sensor_num, char *temp_sensor,
                          char ***current_max_file, int *current_max_file_num,
                          char ***current_limit_file, int *current_limit_file_num,
                          char ***temp_file, int *temp_file_num);

#endif
