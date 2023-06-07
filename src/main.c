#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "unistd.h"
#include "regex.h"
#include "malloc.h"
#include "pthread.h"
#include "sys/stat.h"

#include "main.h"
#include "read_option.h"
#include "some_ctrl.h"
#include "printf_with_time.h"
#include "value_set.h"
#include "foreground_app.h"

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
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
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

void check_read_file(char *file)
{
    if(!access(file, F_OK))
    {
        if(access(file, R_OK))
        {
            chmod(file, 0644);
            if(access(file, R_OK))
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

int main()
{
    FILE *fq;
    char **current_limit_file,**power_supply_dir_list,**power_supply_dir,**thermal_dir,**current_max_file,**temp_file,charge[25],power[10];
    char *temp_tmp,*temp_sensor,*temp_sensor_dir,*buffer,*msg,current_max_char[20],highest_temp_current_char[20],thermal[15],last_appname[100];
    uchar step_charge=1,step_charge_file=0,power_control=1,force_temp=1,has_force_temp=0,current_change=1,battery_status=1,battery_capacity=1,tmp[5]={0};
    int i=0,j=0,temp_sensor_num=100,temp_int=0,power_supply_file_num=0,thermal_file_num=0,current_limit_file_num=0;
    int power_supply_dir_list_num=0,current_max_file_num=0,temp_file_num=0,is_bypass=0;
    uint option_last_modify_time=0;
    regex_t temp_re,current_max_re,current_limit_re;
    regmatch_t temp_pmatch,current_max_pmatch,current_limit_pmatch;
    pthread_t thread1;
    struct stat statbuf;
    printf("作者：酷安@诺鸡鸭\n");
    printf("GitHub开源地址：https://github.com/chase535/turbo-charge\n\n");
    fflush(stdout);
    if(access("/sys/class/power_supply/battery/status", F_OK)) battery_status=0;
    if(access("/sys/class/power_supply/battery/capacity", F_OK)) battery_capacity=0;
    if(!battery_status || !battery_capacity)
    {
        power_control=0;
        if(battery_status && !battery_capacity)
            printf_with_time("由于找不到/sys/class/power_supply/battery/capacity文件，电量控制功能失效！");
        else if(!battery_status && battery_capacity)
            printf_with_time("由于找不到/sys/class/power_supply/battery/status文件，电量控制功能失效，且“伪”旁路供电功能无法根据手机的充电状态而自动启停！");
        else
            printf_with_time("由于找不到/sys/class/power_supply/battery/status和/sys/class/power_supply/battery/capacity文件，电量控制功能失效，且“伪”旁路供电功能无法根据手机的充电状态而自动启停！");
    }
    else
    {
        if(access("/sys/class/power_supply/battery/charging_enabled", F_OK) && access("/sys/class/power_supply/battery/battery_charging_enabled", F_OK) && access("/sys/class/power_supply/battery/input_suspend", F_OK) && access("/sys/class/qcom-battery/restricted_charging", F_OK))
        {
            power_control=0;
            printf_with_time("由于找不到控制手机暂停充电的文件，电量控制功能失效！");
            printf_with_time("目前已知的有关文件有：/sys/class/power_supply/battery/charging_enabled、/sys/class/power_supply/battery/battery_charging_enabled、/sys/class/power_supply/battery/input_suspend、/sys/class/qcom-battery/restricted_charging");
            printf_with_time("如果您知道其他的有关文件，请联系模块制作者！");
        }
    }
    if(!access("/sys/class/power_supply/battery/step_charging_enabled", F_OK)) step_charge_file++;
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
            if(!regexec(&current_limit_re, power_supply_dir_list[j], 1, &current_limit_pmatch, 0))
            {
                current_limit_file[current_limit_file_num]=(char *)calloc(1, sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(current_limit_file[current_limit_file_num], power_supply_dir_list[j]);
                current_limit_file_num++;
            }
            if(!regexec(&current_max_re, power_supply_dir_list[j], 1, &current_max_pmatch, 0))
            {
                current_max_file[current_max_file_num]=(char *)calloc(1, sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(current_max_file[current_max_file_num], power_supply_dir_list[j]);
                current_max_file_num++;
            }
            if(!regexec(&temp_re, power_supply_dir_list[j], 1, &temp_pmatch, 0))
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
        printf_with_time("无法在/sys/class/power_supply中的所有文件夹内找到constant_charge_current_max、fast_charge_current、thermal_input_current文件，有关电流的所有功能（包括“伪”旁路供电功能）失效！");
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
        temp_tmp=(char *)calloc(1, sizeof(char)*15);
        msg=(char *)calloc(1, sizeof(char));
        thermal_file_num=list_dir("/sys/class/thermal", &thermal_dir);
        for(i=0;i < thermal_file_num;i++)
        {
            if(strstr(thermal_dir[i], "thermal_zone") != NULL)
            {
                buffer=(char *)realloc(buffer, sizeof(char)*(strlen(thermal_dir[i])+6));
                sprintf(buffer, "%s/type", thermal_dir[i]);
                if(access(buffer, R_OK)) continue;
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
                line_feed(msg);
                sprintf(buffer, "%s/temp", thermal_dir[i]);
                if(access(buffer, R_OK)) continue;
                fq=fopen(buffer, "rt");
                if(fq != NULL)
                {
                    fgets(temp_tmp, 10, fq);
                    fclose(fq);
                    fq=NULL;
                }
                line_feed(temp_tmp);
                if(atoi(temp_tmp) == 1 || atoi(temp_tmp) == 0 || atoi(temp_tmp) == -1) continue;
                for(j=0;j < TEMP_SENSOR_QUANTITY;j++)
                {
                    if(!strcmp(msg, temp_sensors[j]) && temp_sensor_num > j)
                    {
                        temp_sensor_num=j;
                        temp_sensor_dir=(char *)realloc(temp_sensor_dir, sizeof(char)*(strlen(thermal_dir[i])+1));
                        strcpy(temp_sensor_dir, thermal_dir[i]);
                    }
                }
            }
        }
        free(buffer);
        buffer=NULL;
        free(temp_tmp);
        temp_tmp=NULL;
        free(msg);
        msg=NULL;
        free_celloc_memory(&thermal_dir, thermal_file_num);
        if(temp_sensor_num != 100)
        {
            temp_sensor=(char *)realloc(temp_sensor, sizeof(char)*(strlen(temp_sensor_dir)+6));
            sprintf(temp_sensor, "%s/temp", temp_sensor_dir);
            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "将使用%s温度传感器作为手机温度的获取源。由于每个传感器所处位置不同以及每个手机发热区不同，很可能导致获取到的温度与实际体感温度不同", temp_sensors[temp_sensor_num]);
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
    read_options(&option_last_modify_time, 0, tmp, 0);
    snprintf(current_max_char, 20, "%d", read_one_option("CURRENT_MAX"));
    snprintf(highest_temp_current_char, 20, "%d", read_one_option("HIGHEST_TEMP_CURRENT"));
    printf_with_time("文件检测完毕，程序开始运行");
    set_value("/sys/kernel/fast_charge/force_fast_charge", "1");
    set_value("/sys/class/power_supply/battery/system_temp_level", "1");
    set_value("/sys/class/power_supply/usb/boost_current", "1");
    set_value("/sys/class/power_supply/battery/safety_timer_enabled", "0");
    set_value("/sys/kernel/fast_charge/failsafe", "1");
    set_value("/sys/class/power_supply/battery/allow_hvdcp3", "1");
    set_value("/sys/class/power_supply/usb/pd_allowed", "1");
    set_value("/sys/class/power_supply/battery/input_current_limited", "0");
    set_value("/sys/class/power_supply/battery/input_current_settled", "1");
    set_value("/sys/class/qcom-battery/restrict_chg", "0");
    charge_ctl("1");
    while(1)
    {
        read_options(&option_last_modify_time, 1, tmp, 0);
        snprintf(current_max_char, 20, "%d", read_one_option("CURRENT_MAX"));
        snprintf(highest_temp_current_char, 20, "%d", read_one_option("HIGHEST_TEMP_CURRENT"));
        set_array_value(current_limit_file, current_limit_file_num, "-1");
        if(!battery_status)
        {
            if(step_charge == 1)
            {
                if(read_one_option("STEP_CHARGING_DISABLED") == 1) (atoi(power) < read_one_option("STEP_CHARGING_DISABLED_THRESHOLD"))?step_charge_ctl("1"):step_charge_ctl("0");
                else step_charge_ctl("1");
            }
            else if(step_charge == 2)
                (read_one_option("STEP_CHARGING_DISABLED") == 1)?step_charge_ctl("0"):step_charge_ctl("1");
            if(current_change)
            {
                if(bypass_charge == 1 && !strlen((char *)ForegroundAppName))
                {
                    strcpy((char *)ForegroundAppName, "chase535");
                    pthread_create(&thread1, NULL, get_foreground_appname, NULL);
                    pthread_detach(thread1);
                }
                else if(bypass_charge == 1 && strlen((char *)ForegroundAppName) && !strcmp((char *)ForegroundAppName, "chase535"))
                {
                    bypass_charge_ctl(last_appname, &is_bypass, current_max_file, current_max_file_num);
                    if(is_bypass)
                    {
                        sleep(read_one_option("CYCLE_TIME"));
                        continue;
                    }
                }
                else if(strlen(last_appname)) memset(last_appname, 0, sizeof(last_appname));
            }
            if(current_change) set_array_value(current_max_file, current_max_file_num, current_max_char);
            sleep(read_one_option("CYCLE_TIME"));
            continue;
        }
        if(force_temp && read_one_option("FORCE_TEMP") == 1 && !has_force_temp) has_force_temp=1;
        check_read_file("/sys/class/power_supply/battery/capacity");
        fq=fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fq);
        fclose(fq);
        fq=NULL;
        line_feed(power);
        if(step_charge == 1)
        {
            if(read_one_option("STEP_CHARGING_DISABLED") == 1) (atoi(power) < read_one_option("STEP_CHARGING_DISABLED_THRESHOLD"))?step_charge_ctl("1"):step_charge_ctl("0");
            else step_charge_ctl("1");
        }
        else if(step_charge == 2)
            (read_one_option("STEP_CHARGING_DISABLED") == 1)?step_charge_ctl("0"):step_charge_ctl("1");
        check_read_file("/sys/class/power_supply/battery/status");
        fq=fopen("/sys/class/power_supply/battery/status", "rt");
        fgets(charge, 20, fq);
        fclose(fq);
        fq=NULL;
        line_feed(charge);
        if(strcmp(charge, "Discharging"))
        {
            if(tmp[0] || !tmp[1])
            {
                printf_with_time("充电器已连接");
                tmp[0]=0;
                tmp[1]=1;
            }
            if(force_temp && read_one_option("FORCE_TEMP") == 1) set_array_value(temp_file, temp_file_num, "280");
            else if(has_force_temp) set_temp(temp_sensor, temp_file, temp_file_num, 0);
            if(power_control) powel_ctl(tmp);
            if(current_change)
            {
                if(bypass_charge == 1 && !strlen((char *)ForegroundAppName))
                {
                    pthread_create(&thread1, NULL, get_foreground_appname, NULL);
                    pthread_detach(thread1);
                }
                else if(bypass_charge == 1 && strlen((char *)ForegroundAppName))
                {
                    bypass_charge_ctl(last_appname, &is_bypass, current_max_file, current_max_file_num);
                    if(is_bypass)
                    {
                        sleep(read_one_option("CYCLE_TIME"));
                        continue;
                    }
                }
                else if(strlen(last_appname)) memset(last_appname, 0, sizeof(last_appname));
            }
            if(read_one_option("TEMP_CTRL") == 1 && temp_sensor_num != 100 && current_change)
            {
                check_read_file(temp_sensor);
                fq=fopen(temp_sensor, "rt");
                fgets(thermal, 10, fq);
                fclose(fq);
                fq=NULL;
                line_feed(thermal);
                temp_int=atoi(thermal);
                if(temp_int >= read_one_option("TEMP_MAX")*1000)
                {
                    snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "手机温度大于等于降低充电电流的温度阈值，限制充电电流为%sμA", highest_temp_current_char);
                    printf_with_time(chartmp);
                    while(!is_bypass)
                    {
                        read_options(&option_last_modify_time, 1, tmp, 1);
                        snprintf(current_max_char, 20, "%d", read_one_option("CURRENT_MAX"));
                        snprintf(highest_temp_current_char, 20, "%d", read_one_option("HIGHEST_TEMP_CURRENT"));
                        set_array_value(current_limit_file, current_limit_file_num, "-1");
                        if(force_temp && read_one_option("FORCE_TEMP") == 1 && !has_force_temp) has_force_temp=1;
                        check_read_file(temp_sensor);
                        fq=fopen(temp_sensor, "rt");
                        fgets(thermal, 10, fq);
                        fclose(fq);
                        fq=NULL;
                        line_feed(thermal);
                        temp_int=atoi(thermal);
                        if(tmp[3])
                        {
                            if(temp_int < read_one_option("TEMP_MAX")*1000)
                            {
                                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新的降低充电电流的温度阈值高于旧的温度阈值，且手机温度小于新的温度阈值，恢复充电电流为%sμA", current_max_char);
                                printf_with_time(chartmp);
                                break;
                            }
                            else
                            {
                                snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "新的降低充电电流的温度阈值高于旧的温度阈值，但手机温度大于等于新的温度阈值，限制充电电流为%sμA", highest_temp_current_char);
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
                        if(!strcmp(charge, "Discharging"))
                        {
                            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "充电器断开连接，恢复充电电流为%sμA", current_max_char);
                            printf_with_time(chartmp);
                            tmp[0]=1;
                            tmp[1]=0;
                            break;
                        }
                        if(temp_int <= read_one_option("RECHARGE_TEMP")*1000)
                        {
                            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "手机温度小于等于恢复快充的温度阈值，恢复充电电流为%sμA", current_max_char);
                            printf_with_time(chartmp);
                            break;
                        }
                        if(!read_one_option("TEMP_CTRL"))
                        {
                            snprintf(chartmp, PRINTF_WITH_TIME_MAX_SIZE, "温控关闭，恢复充电电流为%sμA", current_max_char);
                            printf_with_time(chartmp);
                            break;
                        }
                        if(step_charge == 1)
                        {
                            if(read_one_option("STEP_CHARGING_DISABLED") == 1) (atoi(power) < read_one_option("STEP_CHARGING_DISABLED_THRESHOLD"))?step_charge_ctl("1"):step_charge_ctl("0");
                            else step_charge_ctl("1");
                        }
                        else if(step_charge == 2)
                            (read_one_option("STEP_CHARGING_DISABLED") == 1)?step_charge_ctl("0"):step_charge_ctl("1");
                        set_array_value(current_max_file, current_max_file_num, highest_temp_current_char);
                        if(force_temp && read_one_option("FORCE_TEMP") == 1) set_array_value(temp_file, temp_file_num, "280");
                        else if(has_force_temp) set_temp(temp_sensor, temp_file, temp_file_num, 0);
                        if(power_control) powel_ctl(tmp);
                        sleep(read_one_option("CYCLE_TIME"));
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
            if(strlen((char *)ForegroundAppName))
            {
                printf_with_time("手机未在充电状态，“伪”旁路供电功能暂时停用");
                pthread_cancel(thread1);
                memset((void *)ForegroundAppName, 0, sizeof(ForegroundAppName));
            }
            if(step_charge == 1)
            {
                if(read_one_option("STEP_CHARGING_DISABLED") == 1) (atoi(power) < read_one_option("STEP_CHARGING_DISABLED_THRESHOLD"))?step_charge_ctl("1"):step_charge_ctl("0");
                else step_charge_ctl("1");
            }
            else if(step_charge == 2)
                (read_one_option("STEP_CHARGING_DISABLED") == 1)?step_charge_ctl("0"):step_charge_ctl("1");
            if(power_control) powel_ctl(tmp);
            if(has_force_temp)
            {
                if(read_one_option("FORCE_TEMP") == 1) set_temp(temp_sensor, temp_file, temp_file_num, 1);
                else set_temp(temp_sensor, temp_file, temp_file_num, 0);
            }
        }
        sleep(read_one_option("CYCLE_TIME"));
    }
    return 0;
}
