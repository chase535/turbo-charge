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

#define chartmp_size 400

struct tm *get_time(int timezone)
{
    time_t cur_time;
    struct tm *ptm;
    time(&cur_time);
    ptm=gmtime(&cur_time);
    ptm->tm_year+=1900;
    ptm->tm_mon+=1;
    ptm->tm_hour+=timezone;
    if(ptm->tm_hour > 23)
    {
        ptm->tm_hour-=24;
        ptm->tm_mday+=1;
        switch(ptm->tm_mon)
        {
            case 1: case 3: case 5: case 7: case 8: case 10: case 12:
                if(ptm->tm_mday > 31)
                {
                    ptm->tm_mday-=31;
                    ptm->tm_mon+=1;
                }
                break;
            case 2:
                if(((ptm->tm_year%4 == 0) && (ptm->tm_year%100 != 0)) || (ptm->tm_year%400 == 0))
                {
                    if(ptm->tm_mday > 29)
                    {
                        ptm->tm_mday-=29;
                        ptm->tm_mon+=1;
                    }
                }
                else
                {
                    if(ptm->tm_mday > 28)
                    {
                        ptm->tm_mday-=28;
                        ptm->tm_mon+=1;
                    }
                }
                break;
            default:
                if(ptm->tm_mday > 30)
                {
                    ptm->tm_mday-=30;
                    ptm->tm_mon+=1;
                }
                break;
        }
        if(ptm->tm_mon > 12)
        {
            ptm->tm_mon-=12;
            ptm->tm_year+=1;
        }
    }
    return ptm;
}

void printf_plus_time(char *dat)
{
    struct tm *time_get=get_time(8);
    printf("[ %04d.%02d.%02d %02d:%02d:%02d UTC+8 ] %s\n", time_get->tm_year, time_get->tm_mon, time_get->tm_mday, time_get->tm_hour, time_get->tm_min, time_get->tm_sec, dat);
    fflush(stdout);
}

void free_celloc_memory(char ***addr,int num)
{
    if(addr != NULL && *addr != NULL)
    {
        for(int i=0;i<num;i++)
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

void strrpc(char *str, char *oldstr, char *newstr)
{
    char bstr[strlen(str)];
    int i=0;
    memset(bstr,0,sizeof(bstr));
    for(i=0;i<(int)strlen(str);i++)
    {
        if(!strncmp(str+i, oldstr, strlen(oldstr)))
        {
            strcat(bstr,newstr);
            i+=(int)strlen(oldstr)-1;
        }
        else strncat(bstr,str+i,1);
    }
    strcpy(str,bstr);
}

int list_dir(char *path, char ***ppp)
{
    DIR *pDir;
    struct dirent *ent;
    int file_num=0;
    pDir = opendir(path);
    if(pDir != NULL)
    {
        *ppp=(char **)calloc(1,sizeof(char *)*500);
        while ((ent = readdir(pDir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            (*ppp)[file_num]=(char *)calloc(1,sizeof(char)*((strlen(path)+strlen(ent->d_name))+2));
            sprintf((*ppp)[file_num],"%s/%s",path,ent->d_name);
            file_num++;
        }
        closedir(pDir);
        *ppp=(char **)realloc(*ppp,sizeof(char *)*file_num);
    }
    return file_num;
}

void set_value(char *file, char *numb)
{
    FILE *fn;
    if(access(file, F_OK) == 0)
    {
        fn = fopen(file, "wt");
        if(fn != NULL)
        {
            fputs(numb,fn);
            fclose(fn);
            fn = NULL;
        }
        else
        {
            chmod(file, 0644);
            fn = fopen(file, "wt");
            if(fn != NULL)
            {
                fputs(numb,fn);
                fclose(fn);
                fn = NULL;
            }
        }
    }
}

void set_array_value(char **file, int num, char *value)
{
    for(int i=0;i<num;i++) set_value(file[i],value);
}

void line_feed(char *line)
{
    char *p;
    if((p = strchr(line, '\n')) != NULL) *p = '\0';
}

void charge_value(char *i)
{
    set_value("/sys/class/power_supply/battery/charging_enabled", i);
    set_value("/sys/class/power_supply/battery/battery_charging_enabled", i);
    (atoi(i))?set_value("/sys/class/power_supply/battery/input_suspend", "0"):set_value("/sys/class/power_supply/battery/input_suspend", "1");
    (atoi(i))?set_value("/sys/class/qcom-battery/restricted_charging", "0"):set_value("/sys/class/qcom-battery/restricted_charging", "1");
}

void check_read_file(char *file,char chartmp[chartmp_size])
{
    if(file == NULL)
    {
        snprintf(chartmp,chartmp_size,"指针为空，出现异常错误，程序强制退出！");
        printf_plus_time(chartmp);
        exit(789);
    }
    if(access(file, F_OK) == 0)
    {
        if(access(file, R_OK) != 0)
        {
            chmod(file, 0644);
            if(access(file, R_OK) != 0)
            {
                snprintf(chartmp,chartmp_size,"无法读取%s文件，程序强制退出！",file);
                printf_plus_time(chartmp);
                exit(1);
            }
        }
    }
    else
    {
        snprintf(chartmp,chartmp_size,"找不到%s文件，程序强制退出！",file);
        printf_plus_time(chartmp);
        exit(999);
    }
}

void read_option(unsigned int opt_new[10], unsigned int opt_old[10], unsigned char tmp[5], unsigned char num, char chartmp[chartmp_size], unsigned char is_temp_wall)
{
    FILE *fc;
    char options[10][40]={"STEP_CHARGING_DISABLED","TEMP_CTRL","POWER_CTRL","CURRENT_MAX","STEP_CHARGING_DISABLED_THRESHOLD","CHARGE_STOP","CHARGE_START","TEMP_MAX","HIGHEST_TEMP_CURRENT","RECHARGE_TEMP"};
    unsigned char opt;
    struct stat statbuf;
    check_read_file("/data/adb/turbo-charge/option.txt",chartmp);
    stat("/data/adb/turbo-charge/option.txt",&statbuf);
    char option[statbuf.st_size+1];
    fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
    while(fgets(option, 2000, fc) != NULL)
    {
        sscanf(option, "STEP_CHARGING_DISABLED=%u", &opt_new[0]);
        sscanf(option, "TEMP_CTRL=%u", &opt_new[1]);
        sscanf(option, "POWER_CTRL=%u", &opt_new[2]);
        sscanf(option, "CURRENT_MAX=%u", &opt_new[3]);
        sscanf(option, "STEP_CHARGING_DISABLED_THRESHOLD=%u", &opt_new[4]);
        sscanf(option, "CHARGE_STOP=%u", &opt_new[5]);
        sscanf(option, "CHARGE_START=%u", &opt_new[6]);
        sscanf(option, "TEMP_MAX=%u", &opt_new[7]);
        sscanf(option, "HIGHEST_TEMP_CURRENT=%u", &opt_new[8]);
        sscanf(option, "RECHARGE_TEMP=%u", &opt_new[9]);
    }
    fclose(fc);
    fc=NULL;
    if(num)
    {
        for(opt=0;opt<10;opt++)
        {
            if(opt_old[opt] != opt_new[opt])
            {
                snprintf(chartmp,chartmp_size,"%s值发生改变，新%s值为%d",options[opt],options[opt],opt_new[opt]);
                printf_plus_time(chartmp);
                if(opt == 5 && opt_old[opt] < opt_new[opt]) tmp[4]=1;
                if(is_temp_wall == 1 && (opt == 7 && opt_old[opt] < opt_new[opt])) tmp[3]=1;
                opt_old[opt]=opt_new[opt];
            }
        }
    }
    else for(opt=0;opt<10;opt++) opt_old[opt]=opt_new[opt];
}

void powel_ctl(unsigned int opt_new[10], unsigned char tmp[5], char chartmp[chartmp_size])
{
    if(opt_new[2] == 1)
    {
        FILE *fd;
        struct stat statbuf;
        check_read_file("/sys/class/power_supply/battery/capacity",chartmp);
        stat("/sys/class/power_supply/battery/capacity",&statbuf);
        char power[statbuf.st_size+1];
        fd = fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fd);
        fclose(fd);
        fd=NULL;
        line_feed(power);
        if(tmp[2] && tmp[4])
        {
            if(atoi(power) < (int)opt_new[5])
            {
                snprintf(chartmp,chartmp_size,"新的停止充电的电量阈值高于旧的电量阈值，且手机当前电量为%s%%，小于新的电量阈值，恢复充电",power);
                printf_plus_time(chartmp);
                charge_value("1");
                tmp[2]=0;
            }
            else
            {
                snprintf(chartmp,chartmp_size,"新的停止充电的电量阈值高于旧的电量阈值，但手机当前电量为%s%%，大于等于新的电量阈值，停止充电",power);
                printf_plus_time(chartmp);
                charge_value("0");
                tmp[2]=1;
            }
            tmp[4]=0;
        }
        if(atoi(power) >= (int)opt_new[5])
        {
            if(!tmp[2])
            {
                snprintf(chartmp,chartmp_size,"当前电量为%s%%，大于等于停止充电的电量阈值，停止充电",power);
                printf_plus_time(chartmp);
                tmp[2]=1;
            }
            charge_value("0");
        }
        if(atoi(power) <= (int)opt_new[6])
        {
            if(tmp[2])
            {
                snprintf(chartmp,chartmp_size,"当前电量为%s%%，小于等于恢复充电的电量阈值，恢复充电",power);
                printf_plus_time(chartmp);
                tmp[2]=0;
            }
            charge_value("1");
        }
    }
    else
    {
        if(tmp[2])
        {
            printf_plus_time("电量控制关闭，恢复充电");
            tmp[2]=0;
        }
        charge_value("1");
    }
}

int main()
{
    FILE *fq;
    char **power_supply_dir_list,**power_supply_dir,**thermal_dir,**current_max_file,**temp_file,charge[25],power[10],chartmp[chartmp_size],current_max_char[20];
    char *conn_therm,*buffer,*msg,highest_temp_current_char[20],thermal[15],bat_temp_tmp[1],bat_temp[6];
    unsigned char tmp[5]={0,0,0,0,0},num=0,fu=0,bat_temp_size=0,step_charge=1,power_control=1,force_temp=1,current_change=1,battery_status=1,battery_capacity=1;
    int i=0,j=0,temp_int=0,power_supply_file_num=0,thermal_file_num=0,power_supply_dir_list_num=0,current_max_file_num=0,temp_file_num=0;;
    unsigned int opt_old[10]={0,0,0,0,0,0,0,0,0,0},opt_new[10]={0,0,0,0,0,0,0,0,0,0};
    regex_t temp_re,current_max_re;
    regmatch_t temp_pmatch,current_max_pmatch;
    struct stat statbuf;
    printf("作者：酷安@诺鸡鸭\n");
    printf("QQ群：738661277\n");
    printf("GitHub开源地址：https://github.com/chase535/turbo-charge\n");
    fflush(stdout);
    if(access("/sys/class/power_supply/battery/status", F_OK) != 0) battery_status=0;
    if(access("/sys/class/power_supply/battery/capacity", F_OK) != 0) battery_capacity=0;
    if(!battery_status || !battery_capacity)
    {
        power_control=0;
        if(battery_status && !battery_capacity)
            printf_plus_time("由于找不到/sys/class/power_supply/battery/capacity文件，电量控制功能失效！");
        else if(!battery_status && battery_capacity)
            printf_plus_time("由于找不到/sys/class/power_supply/battery/status文件，电量控制功能失效！");
        else
            printf_plus_time("由于找不到/sys/class/power_supply/battery/status和/sys/class/power_supply/battery/capacity文件，电量控制功能失效！");
    }
    else
    {
        if(access("/sys/class/power_supply/battery/charging_enabled", F_OK) != 0 && access("/sys/class/power_supply/battery/battery_charging_enabled", F_OK) != 0 && access("/sys/class/power_supply/battery/input_suspend", F_OK) != 0 && access("/sys/class/qcom-battery/restricted_charging", F_OK) != 0)
        {
            power_control=0;
            printf_plus_time("由于找不到控制手机暂停充电的文件，电量控制功能失效！");
            printf_plus_time("目前已知的有关文件有：/sys/class/power_supply/battery/charging_enabled、/sys/class/power_supply/battery/battery_charging_enabled、/sys/class/power_supply/battery/input_suspend、/sys/class/qcom-battery/restricted_charging");
            printf_plus_time("如果您知道其他的有关文件，请联系模块制作者！");
        }
    }
    if(access("/sys/class/power_supply/battery/step_charging_enabled", F_OK) != 0 || !battery_capacity)
    {
        if(access("/sys/class/power_supply/battery/step_charging_enabled", F_OK) == 0 && !battery_capacity)
        {
            step_charge=2;
            printf_plus_time("由于找不到/sys/class/power_supply/battery/capacity文件，阶梯式充电无法根据电量进行开关，此时若在配置中关闭阶梯式充电，则无论电量多少，阶梯式充电都会关闭！");
        }
        else
        {
            step_charge=0;
            printf_plus_time("由于找不到/sys/class/power_supply/battery/step_charging_enabled文件，阶梯式充电控制的所有功能失效！");
        }
    }
    regcomp(&current_max_re,".*constant_charge_current_max$|.*constant_charge_current$|.*fast_charge_current$|.*thermal_input_current$",REG_EXTENDED|REG_NOSUB);
    regcomp(&temp_re,".*temp$",REG_EXTENDED|REG_NOSUB);
    power_supply_file_num=list_dir("/sys/class/power_supply", &power_supply_dir);
    current_max_file=(char **)calloc(1,sizeof(char *)*100);
    temp_file=(char **)calloc(1,sizeof(char *)*100);
    for(i=0;i<power_supply_file_num;i++)
    {
        power_supply_dir_list_num=list_dir(power_supply_dir[i], &power_supply_dir_list);
        for(j=0;j<power_supply_dir_list_num;j++)
        {
            if(regexec(&current_max_re, power_supply_dir_list[j],1,&current_max_pmatch,0)==0)
            {
                current_max_file[current_max_file_num]=(char *)calloc(1,sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(current_max_file[current_max_file_num],power_supply_dir_list[j]);
                current_max_file_num++;
            }
            if(regexec(&temp_re, power_supply_dir_list[j],1,&temp_pmatch,0)==0)
            {
                temp_file[temp_file_num]=(char *)calloc(1,sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(temp_file[temp_file_num],power_supply_dir_list[j]);
                temp_file_num++;
            }
        }
        free_celloc_memory(&power_supply_dir_list,power_supply_dir_list_num);
    }
    current_max_file=(char **)realloc(current_max_file, sizeof(char *)*current_max_file_num);
    temp_file=(char **)realloc(temp_file, sizeof(char *)*temp_file_num);
    free_celloc_memory(&power_supply_dir,power_supply_file_num);
    if(!current_max_file_num)
    {
        current_change=0;
        printf_plus_time("无法在/sys/class/power_supply中的所有文件夹内找到constant_charge_current_max、constant_charge_current、thermal_input_current、fast_charge_current文件，有关电流的所有功能失效！");
    }
    if(!battery_status || !temp_file_num)
    {
        force_temp=0;
        if(battery_status && !temp_file_num)
            printf_plus_time("无法在/sys/class/power_supply中的所有文件夹内找到文件名以temp结尾的文件，充电时强制显示28℃功能失效！");
        else if(!battery_status && temp_file_num)
            printf_plus_time("由于找不到/sys/class/power_supply/battery/status文件，充电时强制显示28℃功能失效！");
        else
            printf_plus_time("由于找不到/sys/class/power_supply/battery/status文件以及无法在/sys/class/power_supply中的所有文件夹内找到文件名以temp结尾的文件，充电时强制显示28℃功能失效！");
    }
    conn_therm=(char *)calloc(1,5);
    strcpy(conn_therm,"none");
    if(force_temp || current_change)
    {
        thermal_file_num=list_dir("/sys/class/thermal", &thermal_dir);
        for(i=0;i<thermal_file_num;i++)
        {
            if(strstr(thermal_dir[i],"thermal_zone")!=NULL)
            {
                buffer=(char *)calloc(1,sizeof(char)*(strlen(thermal_dir[i])+6));
                sprintf(buffer, "%s/type", thermal_dir[i]);
                if(access(buffer, R_OK) != 0) continue;
                stat(buffer,&statbuf);
                fq = fopen(buffer, "rt");
                if(fq != NULL)
                {
                    msg=(char *)calloc(1,sizeof(char)*(statbuf.st_size+1));
                    fgets(msg, statbuf.st_size+1, fq);
                    fclose(fq);
                    fq=NULL;
                }
                else continue;
                if(msg != NULL)
                {
                    line_feed(msg);
                    if(strcmp(msg, "conn_therm") == 0)
                    {
                        strrpc(buffer, "type", "temp");
                        conn_therm=(char *)realloc(conn_therm,sizeof(char)*(strlen(buffer)+1));
                        strcpy(conn_therm, buffer);
                    }
                    free(msg);
                    free(buffer);
                    msg=NULL;
                    buffer=NULL;
                    if(strcmp(conn_therm,"none") != 0) break;
                }
            }
        }
        free_celloc_memory(&thermal_dir,thermal_file_num);
        if(strcmp(conn_therm,"none") == 0)
        {
            if(force_temp)
            {
                printf_plus_time("由于找不到conn_therm温度传感器，温度控制及充电时强制显示28℃功能失效！");
                force_temp=0;
            }
            else printf_plus_time("由于找不到conn_therm温度传感器，温度控制功能失效！");
            if(!step_charge && !power_control && !force_temp && !current_change)
            {
                printf_plus_time("所有的所需文件均不存在，完全不适配此手机，程序强制退出！");
                exit(800);
            }
        }
        else check_read_file(conn_therm,chartmp);
    }
    else
    {
        if(!step_charge && !power_control && !force_temp && !current_change)
        {
            printf_plus_time("所有的所需文件均不存在，完全不适配此手机，程序强制退出！");
            exit(1000);
        }
    }
    if(current_change)
    {
        for(i=0;i<current_max_file_num;i++)
        {
            snprintf(chartmp,chartmp_size,"找到电流文件：%s",current_max_file[i]);
            printf_plus_time(chartmp);
        }
    }
    if(force_temp)
    {
        for(i=0;i<temp_file_num;i++)
        {
            snprintf(chartmp,chartmp_size,"找到温度文件：%s",temp_file[i]);
            printf_plus_time(chartmp);
        }
    }
    check_read_file("/data/adb/turbo-charge/option.txt",chartmp);
    printf_plus_time("文件检测完毕，程序开始运行");
    charge_value("1");
    while(1)
    {
        read_option(opt_new, opt_old, tmp, num, chartmp, 0);
        snprintf(current_max_char,20,"%u",opt_new[3]);
        snprintf(highest_temp_current_char,20,"%u",opt_new[8]);
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
        if(!battery_status)
        {
            if(current_change) set_array_value(current_max_file,current_max_file_num,current_max_char);
            if(step_charge == 1)
            {
                if(opt_new[0] == 1) (atoi(power) < (int)opt_new[4])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
                else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
            }
            else if(step_charge == 2)
                (opt_new[0] == 1)?set_value("/sys/class/power_supply/battery/step_charging_enabled", "0"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
            sleep(5);
            continue;
        }
        check_read_file("/sys/class/power_supply/battery/capacity",chartmp);
        fq = fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fq);
        fclose(fq);
        fq=NULL;
        line_feed(power);
        if(step_charge == 1)
        {
            if(opt_new[0] == 1) (atoi(power) < (int)opt_new[4])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
            else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
        }
        else if(step_charge == 2)
            (opt_new[0] == 1)?set_value("/sys/class/power_supply/battery/step_charging_enabled", "0"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
        check_read_file("/sys/class/power_supply/battery/status",chartmp);
        fq = fopen("/sys/class/power_supply/battery/status", "rt");
        fgets(charge, 20, fq);
        fclose(fq);
        fq=NULL;
        line_feed(charge);
        if(strcmp(charge, "Discharging") != 0)
        {
            if(tmp[0] || !tmp[1])
            {
                printf_plus_time("充电器已连接");
                tmp[0]=0;
                tmp[1]=1;
            }
            if(force_temp) set_array_value(temp_file,temp_file_num,"280");
            if(power_control) powel_ctl(opt_new, tmp, chartmp);
            if(opt_new[1] == 1 && current_change)
            {
                if(strcmp(conn_therm,"none") != 0)
                {
                    check_read_file(conn_therm,chartmp);
                    fq = fopen(conn_therm, "rt");
                    fgets(thermal, 10, fq);
                    fclose(fq);
                    fq=NULL;
                    line_feed(thermal);
                    temp_int = atoi(thermal);
                    if(temp_int >= ((int)opt_new[7])*1000)
                    {
                        snprintf(chartmp,chartmp_size,"手机温度大于等于降低充电电流的温度阈值，限制充电电流为%dμA",opt_new[8]);
                        printf_plus_time(chartmp);
                        while(1)
                        {
                            read_option(opt_new, opt_old, tmp, num, chartmp, 1);
                            snprintf(current_max_char,20,"%u",opt_new[3]);
                            snprintf(highest_temp_current_char,20,"%u",opt_new[8]);
                            check_read_file(conn_therm,chartmp);
                            fq = fopen(conn_therm, "rt");
                            fgets(thermal, 300, fq);
                            fclose(fq);
                            fq=NULL;
                            line_feed(thermal);
                            temp_int = atoi(thermal);
                            if(tmp[3] == 1)
                            {
                                if(temp_int < ((int)opt_new[7])*1000)
                                {
                                    snprintf(chartmp,chartmp_size,"新的降低充电电流的温度阈值高于旧的温度阈值，且手机温度小于新的温度阈值，恢复充电电流为%dμA",opt_new[3]);
                                    printf_plus_time(chartmp);
                                    break;
                                }
                                else
                                {
                                    snprintf(chartmp,chartmp_size,"新的降低充电电流的温度阈值高于旧的温度阈值，但手机温度大于等于新的温度阈值，限制充电电流为%dμA",opt_new[8]);
                                    printf_plus_time(chartmp);
                                }
                                tmp[3]=0;
                            }
                            check_read_file("/sys/class/power_supply/battery/status",chartmp);
                            fq = fopen("/sys/class/power_supply/battery/status", "rt");
                            fgets(charge, 20, fq);
                            fclose(fq);
                            fq=NULL;
                            line_feed(charge);
                            if(strcmp(charge, "Discharging") == 0)
                            {
                                snprintf(chartmp,chartmp_size,"充电器断开连接，恢复充电电流为%dμA",opt_new[3]);
                                printf_plus_time(chartmp);
                                tmp[0]=1;
                                tmp[1]=0;
                                break;
                            }
                            if(temp_int <= ((int)opt_new[9])*1000)
                            {
                                snprintf(chartmp,chartmp_size,"手机温度小于等于恢复快充的温度阈值，恢复充电电流为%dμA",opt_new[3]);
                                printf_plus_time(chartmp);
                                break;
                            }
                            if(opt_new[1] == 0)
                            {
                                snprintf(chartmp,chartmp_size,"温控关闭，恢复充电电流为%dμA",opt_new[3]);
                                printf_plus_time(chartmp);
                                break;
                            }
                            if(step_charge == 1)
                            {
                                if(opt_new[0] == 1) (atoi(power) < (int)opt_new[4])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
                                else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
                            }
                            else if(step_charge == 2)
                                (opt_new[0] == 1)?set_value("/sys/class/power_supply/battery/step_charging_enabled", "0"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
                            set_array_value(current_max_file,current_max_file_num,highest_temp_current_char);
                            if(force_temp) set_array_value(temp_file,temp_file_num,"280");
                            if(power_control) powel_ctl(opt_new, tmp, chartmp);
                            sleep(5);
                        }
                    }
                }
            }
            if(current_change) set_array_value(current_max_file,current_max_file_num,current_max_char);
        }
        else
        {
            if(!tmp[1] && !tmp[0])
            {
                printf_plus_time("充电器未连接");
                tmp[0]=1;
            }
            else if(!tmp[0])
            {
                printf_plus_time("充电器断开连接");
                tmp[0]=1;
            }
            if(step_charge == 1)
            {
                if(opt_new[0] == 1) (atoi(power) < (int)opt_new[4])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
                else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
            }
            else if(step_charge == 2)
                (opt_new[0] == 1)?set_value("/sys/class/power_supply/battery/step_charging_enabled", "0"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
            if(force_temp)
            {
                check_read_file(conn_therm,chartmp);
                fq = fopen(conn_therm, "rt");
                fgets(thermal, 10, fq);
                fclose(fq);
                fq=NULL;
                line_feed(thermal);
                temp_int=atoi(thermal);
                fu=0;
                if(temp_int<0)
                {
                    temp_int=abs(temp_int);
                    fu=1;
                }
                snprintf(bat_temp,4,"%05d",temp_int);
                if(strcmp(bat_temp,"000")==0) sprintf(bat_temp,"0");
                else
                {
                    for(bat_temp_tmp[0]=bat_temp[0];atoi(bat_temp_tmp)==0;bat_temp_tmp[0]=bat_temp[0])
                    {
                        for(bat_temp_size=0;bat_temp_size<5;bat_temp_size++) bat_temp[bat_temp_size]=bat_temp[bat_temp_size+1];
                        bat_temp[5]='\0';
                    }
                }
                if(fu)
                {
                    sprintf(bat_temp,"-%s",bat_temp);
                    set_array_value(temp_file,temp_file_num,bat_temp);
                }
                else (temp_int >= 55000)?set_array_value(temp_file,temp_file_num,"280"):set_array_value(temp_file,temp_file_num,bat_temp);
            }
        }
        sleep(5);
    }
    return 0;
}
