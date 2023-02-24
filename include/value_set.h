#ifndef _VALUE_SET_H
#define _VALUE_SET_H

#include "main.h"

#include "sys/stat.h"
#include "unistd.h"

void set_value(char *file, char *numb);
void set_array_value(char **file, int num, char *value);

#endif
