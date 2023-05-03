#include "stdio.h"
#include "string.h"
#include "sys/stat.h"
#include "unistd.h"

#include "value_set.h"

void set_value(char *file, char *numb)
{
    if(!access(file, F_OK))
    {
        FILE *fn;
        struct stat statbuf;
        stat(file, &statbuf);
        char content[statbuf.st_size+1];
        fn=fopen(file, "rt+");
        if(fn != NULL) goto write_data;
        else
        {
            chmod(file, 0644);
            fn=fopen(file, "rt+");
            if(fn != NULL)
            {
                write_data:
                fgets(content, statbuf.st_size+1, fn);
                line_feed(content);
                if(strcmp(content, numb)) fputs(numb, fn);
                fclose(fn);
                fn=NULL;
            } 
        }
    }
}

void set_array_value(char **file, int num, char *value)
{
    for(int i=0;i < num;i++) set_value(file[i], value);
}

void set_temp(char *temp_sensor, char **temp_file, int temp_file_num, uchar tempwall)
{
    FILE *fq;
    char thermal[15],bat_temp[6];
    int temp_int=0;
    check_read_file(temp_sensor);
    fq=fopen(temp_sensor, "rt");
    fgets(thermal, 10, fq);
    fclose(fq);
    fq=NULL;
    line_feed(thermal);
    temp_int=atoi(thermal);
    (temp_int < 0)?snprintf(bat_temp, 5, "%06d", temp_int):snprintf(bat_temp, 4, "%05d", temp_int);
    temp_int=atoi(bat_temp);
    snprintf(bat_temp, 6, "%d", temp_int);
    if(tempwall) (temp_int >= 450)?set_array_value(temp_file, temp_file_num, "280"):set_array_value(temp_file, temp_file_num, bat_temp);
    else set_array_value(temp_file, temp_file_num, bat_temp);
}
