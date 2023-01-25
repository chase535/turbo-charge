#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"

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
    char childpath[600];
    *ppp=(char**)malloc(sizeof(char *) * 1000);
    pDir = opendir(path);
    while ((ent = readdir(pDir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        sprintf(childpath,"%s/%s",path,ent->d_name);
        (*ppp)[file_num]=(char *)malloc(sizeof(char) * (strlen(childpath) + 1));
        strcpy((*ppp)[file_num],childpath);
        file_num++;
    }
    closedir(pDir);
    return file_num;
}

void set_value(char *file, char *numb)
{
    FILE *fn;
    if(access(file, F_OK) == 0)
    {
        chmod(file, 0777);
        fn = fopen(file, "wt");
        if(fn != NULL)
        {
            fputs(numb,fn);
            fclose(fn);
            fn = NULL;
        }
    }
}

void line_feed(char *line)
{
    char *p;
    if((p = strchr(line, '\n')) != NULL) *p = '\0';
}

void list_dir_set_value(char **file_dir, char *file_name, int file_num, char *value)
{
    int i;
    char file[100];
    for(i=0;i<file_num;i++)
    {
        sprintf(file, "%s/%s", file_dir[i], file_name);
        if(access(file, F_OK) != 0) continue;
        set_value(file, value);
    }
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

void check_file(char *file)
{
    if(access(file, F_OK) != 0)
    {
        printf("无法找到%s文件，程序强制退出！\n", file);
        exit(999);
    }
}

void list_dir_check_file(char *file_dir, char *file_name)
{
    int i,j=0,file_num;
    char file[100], **dir;
    file_num=list_dir(file_dir, &dir);
    for(i=0;i<file_num;i++)
    {
        sprintf(file, "%s/%s", dir[i], file_name);
        if(access(file, F_OK) == 0) j++;
    }
    if(j == 0)
    {
        printf("无法在%s中的所有文件夹内找到%s文件，程序强制退出！\n",file_dir , file_name);
        exit(999);
    }
}

int main()
{
    FILE *fq,*fm,*fc,*fd,*fe;
    char **power_supply_dir,**thermal_dir,done[20],charge[25],power[10],current_max[20],highest_temp_current[20],buffer[100],conn_therm[100]="none",msg[20],thermal[15],option[1010],asdf[15],bat_temp_tmp[1],bat_temp[6];
    int power_supply_file_num,thermal_file_num,temp_int,bat_temp_size,asdf_int,i,fu,qwer=0,temp_ctrl,power_ctrl,charge_start,charge_stop,recharge_temp,temp_max;
    check_file("/sys/class/power_supply/battery/step_charging_enabled");
    check_file("/sys/class/power_supply/battery/status");
    check_file("/sys/class/power_supply/battery/current_now");
    check_file("/sys/class/power_supply/battery/capacity");
    list_dir_check_file("/sys/class/power_supply", "temp");
    list_dir_check_file("/sys/class/power_supply", "constant_charge_current_max");
    thermal_file_num=list_dir("/sys/class/thermal", &thermal_dir);
    for(i=0;i<thermal_file_num;i++)
    {
        if(strstr(thermal_dir[i],"thermal_zone")!=NULL)
        {
            sprintf(buffer, "%s/type", thermal_dir[i]);
            if(access(buffer, R_OK) != 0) continue;
            fq = fopen(buffer, "rt");
            if(fq != NULL)
            {
                fgets(msg, 100, fq);
                fclose(fq);
                fq=NULL;
            }
            else continue;
            line_feed(msg);
            if(strcmp(msg, "conn_therm") == 0)
            {
                strrpc(buffer, "type", "temp");
                strcpy(conn_therm, buffer);
            }
        }
    }
    if(strcmp(conn_therm, "none") == 0 || access(conn_therm, R_OK) != 0)
    {
        printf("获取温度失败！\n");
        exit(2);
    }
    power_supply_file_num=list_dir("/sys/class/power_supply", &power_supply_dir);
    charge_value("1");
    set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
    set_value("/sys/kernel/fast_charge/force_fast_charge", "1");
    set_value("/sys/class/power_supply/battery/system_temp_level", "1");
    set_value("/sys/kernel/fast_charge/failsafe", "1");
    set_value("/sys/class/power_supply/battery/allow_hvdcp3", "1");
    set_value("/sys/class/power_supply/usb/pd_allowed", "1");
    set_value("/sys/class/power_supply/battery/subsystem/usb/pd_allowed", "1");
    set_value("/sys/class/power_supply/battery/input_current_limited", "0");
    set_value("/sys/class/power_supply/battery/input_current_settled", "1");
    set_value("/sys/class/qcom-battery/restrict_chg", "0");
    while(1)
    {
        if(access("/data/adb/turbo-charge/option.txt", R_OK) != 0)
        {
            printf("配置文件丢失！\n");
            exit(1);
        }
        fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
        while(fgets(option, 1000, fc) != NULL)
        {
            sscanf(option, "TEMP_CTRL=%d", &temp_ctrl);
            sscanf(option, "POWER_CTRL=%d", &power_ctrl);
            sscanf(option, "CHARGE_START=%d", &charge_start);
            sscanf(option, "CHARGE_STOP=%d", &charge_stop);
            sscanf(option, "CURRENT_MAX=%s", current_max);
            sscanf(option, "TEMP_MAX=%d", &temp_max);
            sscanf(option, "HIGHEST_TEMP_CURRENT=%s", highest_temp_current);
            sscanf(option, "RECHARGE_TEMP=%d", &recharge_temp);
        }
        fclose(fc);
        fc=NULL;
        if(access("/sys/class/power_supply/battery/status", R_OK) != 0)
        {
            printf("读取充电状态失败！\n");
            exit(10);
        }
        fd = fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fd);
        fclose(fd);
        fd=NULL;
        line_feed(power);
        if(access("/sys/class/power_supply/battery/step_charging_enabled", F_OK) != 0)
        {
            printf("向阶梯充电文件写入数据失败！\n");
            exit(600);
        }
        (atoi(power) < 30)?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
        fe = fopen("/sys/class/power_supply/battery/status", "rt");
        fgets(charge, 20, fe);
        fclose(fe);
        fe=NULL;
        line_feed(charge);
        if(strcmp(charge, "Charging") == 0 || strcmp(charge, "Full") == 0)
        {
            if(access(conn_therm, R_OK) != 0)
            {
                printf("获取温度失败！\n");
                exit(220);
            }
            list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, "280");
            if(power_ctrl == 1)
            {
                if(access("/sys/class/power_supply/battery/capacity", R_OK) != 0)
                {
                    printf("获取电量信息失败！\n");
                    exit(40);
                }
                fd = fopen("/sys/class/power_supply/battery/capacity", "rt");
                fgets(power, 5, fd);
                fclose(fd);
                fd=NULL;
                line_feed(power);
                if(atoi(power) >= charge_stop)
                {
                    if(charge_stop == 100)
                    {
                        if(access("/sys/class/power_supply/battery/current_now", R_OK) != 0)
                        {
                            printf("获取电流信息失败！\n");
                            exit(50);
                        }
                        fm = fopen("/sys/class/power_supply/battery/current_now", "rt");
                        fgets(done, 15, fm);
                        fclose(fm);
                        fm=NULL;
                        line_feed(done);
                        if(atoi(done) == 0)
                        {
                            charge_value("0");
                            qwer = 1;
                        }
                    }
                    else
                    {
                        charge_value("0");
                        qwer = 1;
                    }
                }
                else if(atoi(power) <= charge_start)
                {
                    charge_value("1");
                    qwer = 0;
                }
            }
            else
            {
                if(qwer == 1)
                {
                    charge_value("1");
                    qwer = 0;
                }
            }
            if(temp_ctrl == 1)
            {
                if(access(conn_therm, R_OK) != 0)
                {
                    printf("获取温度失败！\n");
                    exit(60);
                }
                fm = fopen(conn_therm, "rt");
                fgets(thermal, 10, fm);
                fclose(fm);
                fm=NULL;
                line_feed(thermal);
                temp_int = atoi(thermal);
                if(temp_int > temp_max*1000)
                {
                    while(temp_int > recharge_temp*1000)
                    {
                        if(access(conn_therm, R_OK) != 0)
                        {
                            printf("获取温度失败！\n");
                            exit(70);
                        }
                        if(access("/sys/class/power_supply/battery/status", R_OK) != 0)
                        {
                            printf("读取充电状态失败！\n");
                            exit(80);
                        }
                        fe = fopen("/sys/class/power_supply/battery/status", "rt");
                        fgets(charge, 20, fe);
                        fclose(fe);
                        fe=NULL;
                        line_feed(charge);
                        if(strcmp(charge, "Charging") != 0 || strcmp(charge, "Full") != 0) break;
                        list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, "280");
                        if(access(conn_therm, R_OK) != 0)
                        {
                            printf("获取温度失败！\n");
                            exit(90);
                        }
                        fm = fopen(conn_therm, "rt");
                        fgets(thermal, 300, fm);
                        fclose(fm);
                        fm=NULL;
                        line_feed(thermal);
                        temp_int = atoi(thermal);
                        if(access("/data/adb/turbo-charge/option.txt", R_OK) != 0)
                        {
                            printf("配置文件丢失！\n");
                            exit(100);
                        }
                        fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
                        while(fgets(option, 1000, fc) != NULL)
                        {
                            sscanf(option, "TEMP_CTRL=%d", &temp_ctrl);
                            sscanf(option, "CURRENT_MAX=%s", current_max);
                            sscanf(option, "TEMP_MAX=%d", &temp_max);
                            sscanf(option, "HIGHEST_TEMP_CURRENT=%s", highest_temp_current);
                            sscanf(option, "RECHARGE_TEMP=%d", &recharge_temp);
                        }
                        fclose(fc);
                        fc=NULL;
                        if(temp_ctrl == 0) break;
                        list_dir_set_value(power_supply_dir, "constant_charge_current_max", power_supply_file_num, highest_temp_current);
                        list_dir_set_value(power_supply_dir, "current_max", power_supply_file_num, highest_temp_current);
                        set_value("/sys/class/power_supply/usb/pd_current_max", highest_temp_current);
                        set_value("/sys/class/power_supply/usb/passthrough_curr_max", highest_temp_current);
                        sleep(1);
                    }
                }
            }
            list_dir_set_value(power_supply_dir, "constant_charge_current_max", power_supply_file_num, current_max);
            list_dir_set_value(power_supply_dir, "current_max", power_supply_file_num, current_max);
            set_value("/sys/class/power_supply/usb/pd_current_max", current_max);
            set_value("/sys/class/power_supply/usb/passthrough_curr_max", current_max);
        }
        else
        {
            if(access(conn_therm, R_OK) != 0)
            {
                printf("获取温度失败！\n");
                exit(110);
            }
            fm = fopen(conn_therm, "rt");
            fgets(asdf, 10, fm);
            fclose(fm);
            fm=NULL;
            line_feed(asdf);
            asdf_int=atoi(asdf);
            fu=0;
            if(asdf_int<0)
            {
                asdf_int=abs(asdf_int);
                fu=1;
            }
            snprintf(bat_temp,4,"%05d",asdf_int);
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
                list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, bat_temp);
            }
            else (asdf_int >= 55000)?list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, "280"):list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, bat_temp);
        }
        sleep(1);
    }
    return 0;
}
