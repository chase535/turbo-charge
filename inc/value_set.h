#ifndef _VALUE_SET_H
#define _VALUE_SET_H

#include "main.h"

void set_value(char *file, char *numb);
void set_array_value(char **file, int num, char *value);
void set_temp(char *temp_sensor, char **temp_file, int temp_file_num, uchar tempwall);

#endif
