#ifndef _MAIN_H
#define _MAIN_H

#include "global.h"

typedef unsigned char uchar;
typedef unsigned int uint;

int list_dir(char *path, char ***ppp);
void line_feed(char *line);
void check_read_file(char *file);
void set_temp(char *temp_sensor, char **temp_file, int temp_file_num, uchar tempwall);

#endif
