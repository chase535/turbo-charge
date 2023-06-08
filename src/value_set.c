#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "unistd.h"

#include "value_set.h"

/*
向文件写入数据
若文件存在且不可写，尝试修改权限后重新检查是否可写
若文件不存在或修改权限后仍不可写，则跳过本次写操作
*/
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
            //权限0644不得简写成644
            chmod(file, 0644);
            fn=fopen(file, "rt+");
            if(fn != NULL)
            {
                write_data:
                fgets(content, statbuf.st_size+1, fn);
                line_feed(content);
                //先判断文件内容与所写内容是否一致，若不一致才进行写入
                if(strcmp(content, numb)) fputs(numb, fn);
                fclose(fn);
                fn=NULL;
            } 
        }
    }
}

//向二级指针所指向的文件们写入数据
void set_array_value(char **file, int num, char *value)
{
    for(int i=0;i < num;i++) set_value(file[i], value);
}

/*
向二级指针所指向的温度文件们写入温度
tempwall为是否启用超过一定温度时强制向温度文件写入28℃
*/
void set_temp(char *temp_sensor, char **temp_file, int temp_file_num, uchar tempwall)
{
    FILE *fq;
    char thermal[15],bat_temp[6];
    int temp_int=0;
    //读取温度文件
    check_read_file(temp_sensor);
    fq=fopen(temp_sensor, "rt");
    fgets(thermal, 10, fq);
    fclose(fq);
    fq=NULL;
    line_feed(thermal);
    temp_int=atoi(thermal);
    //若温度为正数，则将温度扩展为6位数并取前3位
    //若温度为负数，则将温度扩展为负号+5位数并取负号+前3位
    (temp_int < 0)?snprintf(bat_temp, 5, "%06d", temp_int):snprintf(bat_temp, 4, "%05d", temp_int);
    temp_int=atoi(bat_temp);
    snprintf(bat_temp, 6, "%d", temp_int);
    //如果tempwall为1，则判断温度是否超过指定值，若超过则强制显示28度，否则显示正常温度
    //如果tempwall为0，则永远显示正常温度
    if(tempwall) (temp_int >= 450)?set_array_value(temp_file, temp_file_num, "280"):set_array_value(temp_file, temp_file_num, bat_temp);
    else set_array_value(temp_file, temp_file_num, bat_temp);
}
