#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "unistd.h"
#include "time.h"
#include "regex.h"
#include "malloc.h"
#include "sys/types.h"
#include "sys/stat.h"

#include "main.h"
#include "read_option.h"
#include "printf_with_time.h"

void free_celloc_memory(char ***addr, int num)
{
    if(addr != NULL && *addr != NULL)
    {
        for(int i=0;i < num;i++)
        {
            if((*addr)[i] != NULL)
            {
                free((*addr)[i]);
                (*addr)[i]=NULL;
            }
        }
        free(*addr);
        *addr=NULL;
    }
}

int list_dir(char *path, char ***ppp)
{
    DIR *pDir;
    struct dirent *ent;
    int file_num=0;
    pDir=opendir(path);
    if(pDir != NULL)
    {
        *ppp=(char **)calloc(1, sizeof(char *)*500);
        while((ent=readdir(pDir)) != NULL)
        {
            if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            (*ppp)[file_num]=(char *)calloc(1, sizeof(char)*((strlen(path)+strlen(ent->d_name))+2));
            sprintf((*ppp)[file_num], "%s/%s", path, ent->d_name);
            file_num++;
        }
        closedir(pDir);
        *ppp=(char **)realloc(*ppp, sizeof(char *)*file_num);
    }
    return file_num;
}

void line_feed(char *line)
{
    char *p;
    p=strchr(line, '\r');
    if(p != NULL) *p='\0';
    p=strchr(line, '\n');
    if(p != NULL) *p='\0';
}

void set_value(char *file, char *numb)
{
    if(access(file, F_OK) == 0)
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
                if(strcmp(content, numb) != 0) fputs(numb, fn);
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

void charge_value(char *i)
{
    set_value("/sys/class/power_supply/battery/charging_enabled", i);
    set_value("/sys/class/power_supply/battery/battery_charging_enabled", i);
    if(atoi(i))
    {
        set_value("/sys/class/power_supply/battery/input_suspend", "0");
        set_value("/sys/class/qcom-battery/restricted_charging", "0");
    }
    else
    {
        set_value("/sys/class/power_supply/battery/input_suspend", "1");
        set_value("/sys/class/qcom-battery/restricted_charging", "1");
    }
}

void check_read_file(char *file)
{
    if(file == NULL)
    {
        snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "指针为空，出现异常错误，程序强制退出！");
        printf_with_time(chartmp);
        exit(789);
    }
    if(access(file, F_OK) == 0)
    {
        if(access(file, R_OK) != 0)
        {
            chmod(file, 0644);
            if(access(file, R_OK) != 0)
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "无法读取%s文件，程序强制退出！", file);
                printf_with_time(chartmp);
                exit(1);
            }
        }
    }
    else
    {
        snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "找不到%s文件，程序强制退出！", file);
        printf_with_time(chartmp);
        exit(999);
    }
}

void step_charge_ctl(char *value)
{
    check_read_file("/sys/class/power_supply/battery/step_charging_enabled");
    set_value("/sys/class/power_supply/battery/step_charging_enabled", value);
    set_value("/sys/class/power_supply/battery/sw_jeita_enabled", value);
}

void powel_ctl(void)
{
    if(opt_new[2] == 1)
    {
        FILE *fd;
        struct stat statbuf;
        check_read_file("/sys/class/power_supply/battery/capacity");
        stat("/sys/class/power_supply/battery/capacity", &statbuf);
        char power[statbuf.st_size+1];
        fd=fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fd);
        fclose(fd);
        fd=NULL;
        line_feed(power);
        if(tmp[2] && tmp[4])
        {
            if(atoi(power) < (int)opt_new[5])
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新的停止充电的电量阈值高于旧的电量阈值，且手机当前电量为%s%%，小于新的电量阈值，恢复充电", power);
                printf_with_time(chartmp);
                charge_value("1");
                tmp[2]=0;
            }
            else
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新的停止充电的电量阈值高于旧的电量阈值，但手机当前电量为%s%%，大于等于新的电量阈值，停止充电", power);
                printf_with_time(chartmp);
                charge_value("0");
                tmp[2]=1;
            }
            tmp[4]=0;
        }
        if(atoi(power) >= (int)opt_new[5])
        {
            if(!tmp[2])
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "当前电量为%s%%，大于等于停止充电的电量阈值，停止充电", power);
                printf_with_time(chartmp);
                tmp[2]=1;
            }
            charge_value("0");
        }
        if(atoi(power) <= (int)opt_new[6])
        {
            if(tmp[2])
            {
                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "当前电量为%s%%，小于等于恢复充电的电量阈值，恢复充电", power);
                printf_with_time(chartmp);
                tmp[2]=0;
            }
            charge_value("1");
        }
    }
    else
    {
        if(tmp[2])
        {
            printf_with_time("电量控制关闭，恢复充电");
            tmp[2]=0;
        }
        charge_value("1");
    }
}

int main()
{
    FILE *fq;
    char **current_limit_file,**power_supply_dir_list,**power_supply_dir,**thermal_dir,**current_max_file,**temp_file,charge[25],power[10];
    char *temp_sensor,*temp_sensor_dir,*buffer,*msg,current_max_char[20],highest_temp_current_char[20],thermal[15],bat_temp_tmp[1],bat_temp[6];
    char temp_sensors[12][15]={"lcd_therm","conn_therm","modem_therm","wifi_therm","quiet_therm","mtktsbtsnrpa","mtktsbtsmdpa","mtktsAP","modem-0-usr","modem1_wifi","ddr-usr","cwlan-usr"};
    uchar num=0,negative=0,step_charge=1,step_charge_file=0,power_control=1,force_temp=1,current_change=1,battery_status=1,battery_capacity=1;
    int i=0,j=0,temp_sensor_num=100,temp_int=0,power_supply_file_num=0,thermal_file_num=0,current_limit_file_num=0,power_supply_dir_list_num=0,current_max_file_num=0,temp_file_num=0;
    uint option_last_modify_time=0;
    regex_t temp_re,current_max_re,current_limit_re;
    regmatch_t temp_pmatch,current_max_pmatch,current_limit_pmatch;
    struct stat statbuf;
    printf("作者：酷安@诺鸡鸭\r\n");
    printf("QQ群：738661277\r\n");
    printf("GitHub开源地址：https://github.com/chase535/turbo-charge\r\n\r\n");
    fflush(stdout);
    if(access("/sys/class/power_supply/battery/status", F_OK) != 0) battery_status=0;
    if(access("/sys/class/power_supply/battery/capacity", F_OK) != 0) battery_capacity=0;
    if(!battery_status || !battery_capacity)
    {
        power_control=0;
        if(battery_status && !battery_capacity)
            printf_with_time("由于找不到/sys/class/power_supply/battery/capacity文件，电量控制功能失效！");
        else if(!battery_status && battery_capacity)
            printf_with_time("由于找不到/sys/class/power_supply/battery/status文件，电量控制功能失效！");
        else
            printf_with_time("由于找不到/sys/class/power_supply/battery/status和/sys/class/power_supply/battery/capacity文件，电量控制功能失效！");
    }
    else
    {
        if(access("/sys/class/power_supply/battery/charging_enabled", F_OK) != 0 && access("/sys/class/power_supply/battery/battery_charging_enabled", F_OK) != 0 && access("/sys/class/power_supply/battery/input_suspend", F_OK) != 0 && access("/sys/class/qcom-battery/restricted_charging", F_OK) != 0)
        {
            power_control=0;
            printf_with_time("由于找不到控制手机暂停充电的文件，电量控制功能失效！");
            printf_with_time("目前已知的有关文件有：/sys/class/power_supply/battery/charging_enabled、/sys/class/power_supply/battery/battery_charging_enabled、/sys/class/power_supply/battery/input_suspend、/sys/class/qcom-battery/restricted_charging");
            printf_with_time("如果您知道其他的有关文件，请联系模块制作者！");
        }
    }
    if(access("/sys/class/power_supply/battery/step_charging_enabled", F_OK) == 0) step_charge_file++;
    if(!step_charge_file || !battery_capacity)
    {
        if(step_charge_file && !battery_capacity)
        {
            step_charge=2;
            printf_with_time("由于找不到/sys/class/power_supply/battery/capacity文件，阶梯式充电无法根据电量进行开关，此时若在配置中关闭阶梯式充电，则无论电量多少，阶梯式充电都会关闭！");
        }
        else
        {
            step_charge=0;
            printf_with_time("由于找不到/sys/class/power_supply/battery/step_charging_enabled文件，阶梯式充电控制的所有功能失效！");
        }
    }
    regcomp(&current_max_re, ".*/constant_charge_current_max$|.*/fast_charge_current$|.*/thermal_input_current$", REG_EXTENDED|REG_NOSUB);
    regcomp(&current_limit_re, ".*/thermal_input_current_limit$", REG_EXTENDED|REG_NOSUB);
    regcomp(&temp_re, ".*/temp$", REG_EXTENDED|REG_NOSUB);
    power_supply_file_num=list_dir("/sys/class/power_supply", &power_supply_dir);
    current_limit_file=(char **)calloc(1, sizeof(char *)*100);
    current_max_file=(char **)calloc(1, sizeof(char *)*100);
    temp_file=(char **)calloc(1, sizeof(char *)*100);
    for(i=0;i < power_supply_file_num;i++)
    {
        power_supply_dir_list_num=list_dir(power_supply_dir[i], &power_supply_dir_list);
        for(j=0;j < power_supply_dir_list_num;j++)
        {
            if(regexec(&current_limit_re, power_supply_dir_list[j], 1, &current_limit_pmatch, 0) == 0)
            {
                current_limit_file[current_limit_file_num]=(char *)calloc(1, sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(current_limit_file[current_limit_file_num], power_supply_dir_list[j]);
                current_limit_file_num++;
            }
            if(regexec(&current_max_re, power_supply_dir_list[j], 1, &current_max_pmatch, 0) == 0)
            {
                current_max_file[current_max_file_num]=(char *)calloc(1, sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(current_max_file[current_max_file_num], power_supply_dir_list[j]);
                current_max_file_num++;
            }
            if(regexec(&temp_re, power_supply_dir_list[j], 1, &temp_pmatch, 0) == 0)
            {
                temp_file[temp_file_num]=(char *)calloc(1, sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(temp_file[temp_file_num], power_supply_dir_list[j]);
                temp_file_num++;
            }
        }
        free_celloc_memory(&power_supply_dir_list, power_supply_dir_list_num);
    }
    free_celloc_memory(&power_supply_dir, power_supply_file_num);
    current_limit_file=(char **)realloc(current_limit_file, sizeof(char *)*current_limit_file_num);
    current_max_file=(char **)realloc(current_max_file, sizeof(char *)*current_max_file_num);
    temp_file=(char **)realloc(temp_file, sizeof(char *)*temp_file_num);
    if(!current_max_file_num)
    {
        current_change=0;
        printf_with_time("无法在/sys/class/power_supply中的所有文件夹内找到constant_charge_current_max、fast_charge_current、thermal_input_current文件，有关电流的所有功能失效！");
    }
    if(!battery_status || !temp_file_num)
    {
        force_temp=0;
        if(battery_status && !temp_file_num)
            printf_with_time("无法在/sys/class/power_supply中的所有文件夹内找到temp文件，充电时强制显示28℃功能失效！");
        else if(!battery_status && temp_file_num)
            printf_with_time("由于找不到/sys/class/power_supply/battery/status文件，充电时强制显示28℃功能失效！");
        else
            printf_with_time("由于找不到/sys/class/power_supply/battery/status文件以及无法在/sys/class/power_supply中的所有文件夹内找到temp文件，充电时强制显示28℃功能失效！");
    }
    temp_sensor=(char *)calloc(1, sizeof(char)*5);
    strcpy(temp_sensor, "none");
    if(force_temp || current_change)
    {
        temp_sensor_dir=(char *)calloc(1, sizeof(char));
        buffer=(char *)calloc(1, sizeof(char));
        msg=(char *)calloc(1, sizeof(char));
        thermal_file_num=list_dir("/sys/class/thermal", &thermal_dir);
        for(i=0;i < thermal_file_num;i++)
        {
            if(strstr(thermal_dir[i], "thermal_zone")!=NULL)
            {
                buffer=(char *)realloc(buffer, sizeof(char)*(strlen(thermal_dir[i])+6));
                sprintf(buffer, "%s/type", thermal_dir[i]);
                if(access(buffer, R_OK) != 0) continue;
                stat(buffer, &statbuf);
                fq=fopen(buffer, "rt");
                if(fq != NULL)
                {
                    msg=(char *)realloc(msg, sizeof(char)*(statbuf.st_size+1));
                    fgets(msg, statbuf.st_size+1, fq);
                    fclose(fq);
                    fq=NULL;
                }
                else continue;
                if(msg != NULL)
                {
                    line_feed(msg);
                    for(j=0;j < (int)sizeof(temp_sensors);j++)
                    {
                        if(strcmp(msg, temp_sensors[j]) == 0 && temp_sensor_num > j)
                        {
                            temp_sensor_num=j;
                            temp_sensor_dir=(char *)realloc(temp_sensor_dir, sizeof(char)*(strlen(thermal_dir[i])+1));
                            strcpy(temp_sensor_dir, thermal_dir[i]);
                        }
                    }
                }
            }
        }
        free(buffer);
        buffer=NULL;
        free(msg);
        msg=NULL;
        free_celloc_memory(&thermal_dir, thermal_file_num);
        if(temp_sensor_num != 100)
        {
            temp_sensor=(char *)realloc(temp_sensor, sizeof(char)*(strlen(temp_sensor_dir)+6));
            sprintf(temp_sensor, "%s/temp", temp_sensor_dir);
            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "将使用%s温度传感器作为手机温度的获取源。由于每个传感器所处位置不同以及每个手机发热区不同，很可能导致获取到的温度与实际体感温度不同，请见谅", temp_sensors[temp_sensor_num]);
            printf_with_time(chartmp);
            check_read_file(temp_sensor);
        }
        else
        {
            free(temp_sensor);
            temp_sensor=NULL;
            if(force_temp)
            {
                printf_with_time("由于找不到程序支持的温度传感器，温度控制及充电时强制显示28℃功能失效！");
                force_temp=0;
            }
            else printf_with_time("由于找不到程序支持的温度传感器，温度控制功能失效！");
            if(!step_charge && !power_control && !force_temp && !current_change)
            {
                printf_with_time("所有的所需文件均不存在，完全不适配此手机，程序强制退出！");
                exit(800);
            }
        }
        free(temp_sensor_dir);
        temp_sensor_dir=NULL;
    }
    else
    {
        if(!step_charge && !power_control && !force_temp && !current_change)
        {
            printf_with_time("所有的所需文件均不存在，完全不适配此手机，程序强制退出！");
            exit(1000);
        }
    }
    if(current_change)
    {
        for(i=0;i < current_max_file_num;i++)
        {
            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "找到电流文件：%s", current_max_file[i]);
            printf_with_time(chartmp);
        }
    }
    if(force_temp)
    {
        for(i=0;i < temp_file_num;i++)
        {
            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "找到温度文件：%s", temp_file[i]);
            printf_with_time(chartmp);
        }
    }
    check_read_file(option_file);
    printf_with_time("文件检测完毕，程序开始运行");
    charge_value("1");
    while(1)
    {
        read_option(&option_last_modify_time, num, 0);
        snprintf(current_max_char, 20, "%u", opt_new[3]);
        snprintf(highest_temp_current_char, 20, "%u", opt_new[8]);
        if(!num) num=1;
        set_value("/sys/kernel/fast_charge/force_fast_charge", "1");
        set_value("/sys/class/power_supply/battery/system_temp_level", "1");
        set_value("/sys/class/power_supply/usb/boost_current", "1");
        set_value("/sys/class/power_supply/battery/safety_timer_enabled", "0");
        set_value("/sys/kernel/fast_charge/failsafe", "1");
        set_value("/sys/class/power_supply/battery/allow_hvdcp3", "1");
        set_value("/sys/class/power_supply/usb/pd_allowed", "1");
        set_value("/sys/class/power_supply/battery/subsystem/usb/pd_allowed", "1");
        set_value("/sys/class/power_supply/battery/input_current_limited", "0");
        set_value("/sys/class/power_supply/battery/input_current_settled", "1");
        set_value("/sys/class/qcom-battery/restrict_chg", "0");
        set_array_value(current_limit_file, current_limit_file_num, "-1");
        if(!battery_status)
        {
            if(current_change) set_array_value(current_max_file, current_max_file_num, current_max_char);
            if(step_charge == 1)
            {
                if(opt_new[0] == 1) (atoi(power) < (int)opt_new[4])?step_charge_ctl("1"):step_charge_ctl("0");
                else step_charge_ctl("1");
            }
            else if(step_charge == 2)
                (opt_new[0] == 1)?step_charge_ctl("0"):step_charge_ctl("1");
            sleep(1);
            continue;
        }
        check_read_file("/sys/class/power_supply/battery/capacity");
        fq=fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fq);
        fclose(fq);
        fq=NULL;
        line_feed(power);
        if(step_charge == 1)
        {
            if(opt_new[0] == 1) (atoi(power) < (int)opt_new[4])?step_charge_ctl("1"):step_charge_ctl("0");
            else step_charge_ctl("1");
        }
        else if(step_charge == 2)
            (opt_new[0] == 1)?step_charge_ctl("0"):step_charge_ctl("1");
        check_read_file("/sys/class/power_supply/battery/status");
        fq=fopen("/sys/class/power_supply/battery/status", "rt");
        fgets(charge, 20, fq);
        fclose(fq);
        fq=NULL;
        line_feed(charge);
        if(strcmp(charge, "Discharging") != 0)
        {
            if(tmp[0] || !tmp[1])
            {
                printf_with_time("充电器已连接");
                tmp[0]=0;
                tmp[1]=1;
            }
            if(force_temp) set_array_value(temp_file, temp_file_num, "280");
            if(power_control) powel_ctl();
            if(opt_new[1] == 1 && temp_sensor_num != 100 && current_change)
            {
                check_read_file(temp_sensor);
                fq=fopen(temp_sensor, "rt");
                fgets(thermal, 10, fq);
                fclose(fq);
                fq=NULL;
                line_feed(thermal);
                temp_int=atoi(thermal);
                if(temp_int >= ((int)opt_new[7])*1000)
                {
                    snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "手机温度大于等于降低充电电流的温度阈值，限制充电电流为%dμA", opt_new[8]);
                    printf_with_time(chartmp);
                    while(1)
                    {
                        read_option(&option_last_modify_time, num, 1);
                        snprintf(current_max_char, 20, "%u", opt_new[3]);
                        snprintf(highest_temp_current_char, 20, "%u", opt_new[8]);
                        check_read_file(temp_sensor);
                        fq=fopen(temp_sensor, "rt");
                        fgets(thermal, 300, fq);
                        fclose(fq);
                        fq=NULL;
                        line_feed(thermal);
                        temp_int=atoi(thermal);
                        if(tmp[3] == 1)
                        {
                            if(temp_int < ((int)opt_new[7])*1000)
                            {
                                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新的降低充电电流的温度阈值高于旧的温度阈值，且手机温度小于新的温度阈值，恢复充电电流为%dμA", opt_new[3]);
                                printf_with_time(chartmp);
                                break;
                            }
                            else
                            {
                                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新的降低充电电流的温度阈值高于旧的温度阈值，但手机温度大于等于新的温度阈值，限制充电电流为%dμA", opt_new[8]);
                                printf_with_time(chartmp);
                            }
                            tmp[3]=0;
                        }
                        check_read_file("/sys/class/power_supply/battery/status");
                        fq=fopen("/sys/class/power_supply/battery/status", "rt");
                        fgets(charge, 20, fq);
                        fclose(fq);
                        fq=NULL;
                        line_feed(charge);
                        if(strcmp(charge, "Discharging") == 0)
                        {
                            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "充电器断开连接，恢复充电电流为%dμA", opt_new[3]);
                            printf_with_time(chartmp);
                            tmp[0]=1;
                            tmp[1]=0;
                            break;
                        }
                        if(temp_int <= ((int)opt_new[9])*1000)
                        {
                            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "手机温度小于等于恢复快充的温度阈值，恢复充电电流为%dμA", opt_new[3]);
                            printf_with_time(chartmp);
                            break;
                        }
                        if(opt_new[1] == 0)
                        {
                            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "温控关闭，恢复充电电流为%dμA", opt_new[3]);
                            printf_with_time(chartmp);
                            break;
                        }
                        if(step_charge == 1)
                        {
                            if(opt_new[0] == 1) (atoi(power) < (int)opt_new[4])?step_charge_ctl("1"):step_charge_ctl("0");
                            else step_charge_ctl("1");
                        }
                        else if(step_charge == 2)
                            (opt_new[0] == 1)?step_charge_ctl("0"):step_charge_ctl("1");
                        set_array_value(current_max_file, current_max_file_num, highest_temp_current_char);
                        if(force_temp) set_array_value(temp_file, temp_file_num, "280");
                        if(power_control) powel_ctl();
                        sleep(1);
                    }
                }
            }
            if(current_change) set_array_value(current_max_file, current_max_file_num, current_max_char);
        }
        else
        {
            if(!tmp[1] && !tmp[0])
            {
                printf_with_time("充电器未连接");
                tmp[0]=1;
            }
            else if(!tmp[0])
            {
                printf_with_time("充电器断开连接");
                tmp[0]=1;
            }
            if(step_charge == 1)
            {
                if(opt_new[0] == 1) (atoi(power) < (int)opt_new[4])?step_charge_ctl("1"):step_charge_ctl("0");
                else step_charge_ctl("1");
            }
            else if(step_charge == 2)
                (opt_new[0] == 1)?step_charge_ctl("0"):step_charge_ctl("1");
            if(power_control) powel_ctl();
            if(force_temp)
            {
                check_read_file(temp_sensor);
                fq=fopen(temp_sensor, "rt");
                fgets(thermal, 10, fq);
                fclose(fq);
                fq=NULL;
                line_feed(thermal);
                temp_int=atoi(thermal);
                negative=0;
                if(temp_int < 0)
                {
                    temp_int=abs(temp_int);
                    negative=1;
                }
                snprintf(bat_temp, 4, "%05d", temp_int);
                if(strcmp(bat_temp, "000") == 0) snprintf(bat_temp, sizeof(bat_temp), "0");
                else
                {
                    for(bat_temp_tmp[0]=bat_temp[0];atoi(bat_temp_tmp) == 0;bat_temp_tmp[0]=bat_temp[0])
                    {
                        for(i=0;i < 5;i++) bat_temp[i]=bat_temp[i+1];
                        bat_temp[5]='\0';
                    }
                }
                if(negative)
                {
                    for(i=5;i > 0;i--) bat_temp[i]=bat_temp[i-1];
                    bat_temp[0]='-';
                    set_array_value(temp_file, temp_file_num, bat_temp);
                }
                else (temp_int >= 45000)?set_array_value(temp_file, temp_file_num, "280"):set_array_value(temp_file, temp_file_num, bat_temp);
            }
        }
        sleep(1);
    }
    return 0;
}
