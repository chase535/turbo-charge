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
