#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

void strrpc(char *str, char *oldstr, char *newstr)
{
    char bstr[strlen(str)];
    memset(bstr, 0, sizeof(bstr));
    for(int i = 0; i < strlen(str); i++)
    {
        if(!strncmp(str+i, oldstr, strlen(oldstr)))
        {
            strcat(bstr, newstr);
            i += strlen(oldstr) - 1;
        }
        else
        {
            strncat(bstr, str + i, 1);
        }
    }
    strcpy(str, bstr);
}

void list_dir(char *path, char ***ppp, int *file_num)
{
    DIR *pDir;
    struct dirent *ent;
    int i=0;
    char childpath[600];
    *ppp=(char**)malloc(sizeof(char *) * 1000);
    pDir = opendir(path);
    while ((ent = readdir(pDir)) != NULL)
   {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        sprintf(childpath,"%s/%s",path,ent->d_name);
        (*ppp)[i]=(char *)malloc(sizeof(char) * (strlen(childpath) + 1));
        strcpy((*ppp)[i],childpath);
        i++;
    }
    *file_num=i;
}

void fclose_file(FILE *ffile)
{
    if(ffile != NULL)
    {
        fclose(ffile);
        ffile = NULL;
    }
}

void set_value(char *file, char *numb)
{
    FILE *fn;
    if((file != NULL) && (access(file, W_OK) == 0))
    {
        fn = fopen(file, "wt");
        if(fn != NULL)
        {
            fputs(numb, fn);
            fclose(fn);
            fn = NULL;
        }
    }
}

void line_feed(char *line)
{
    char *p;
    if((p = strchr(line, '\n')) != NULL)
    {
        *p = '\0';
    }
}

void charge_value(char *i)
{
    set_value("/sys/class/power_supply/battery/charging_enabled", i);
    set_value("/sys/class/power_supply/battery/battery_charging_enabled", i);
    if(atoi(i) == 1)
    {
        set_value("/sys/class/power_supply/battery/input_suspend", "0");
        set_value("/sys/class/qcom-battery/restricted_charging", "0");
    }
    else if(atoi(i) == 0)
    {
        set_value("/sys/class/power_supply/battery/input_suspend", "1");
        set_value("/sys/class/qcom-battery/restricted_charging", "1");
    }
}

int main()
{
    FILE *fq,*fm,*fc,*fd,*fe;
    char charge_start,charge_stop,temp_ctrl,power_ctrl,recharge_temp,temp_max,**power_supply_dir,**thermal_dir,done,asdf[310],charge,uevent[3010],power,current_max,highest_temp_current,buffer[100],constants[100],msg[20],thermal[15],temps[100],option[1010];
    int power_supply_file_num,thermal_file_num,i,asdf_int,temp_int,qwer;
    list_dir("/sys/class/thermal", &thermal_dir, &thermal_file_num);
    for(i=0;i<thermal_file_num;i++)
    {
        sprintf(buffer, "%s/type", thermal_dir[i]);
        if(access(buffer, R_OK) != 0) continue;
        fq = fopen(buffer, "rt");
        fgets(msg, 100, fq);
        fclose_file(fq);
        line_feed(msg);
        if(strcmp(msg, "conn_therm") == 0)
        {
            strrpc(buffer, "type", "temp");
            break;
        }
    }
    if(strstr(buffer,"temp") == NULL)
    {
        printf("获取温度失败！请联系模块制作者！");
        exit(2);
    }
    list_dir("/sys/class/power_supply", &power_supply_dir, &power_supply_file_num);
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
            printf("配置文件丢失！请联系模块制作者！");
            exit(1);
        }
        fe = fopen("/sys/class/power_supply/battery/uevent", "rt");
        while (fgets(uevent, 3000, fe) != NULL)
        {
            sscanf(uevent, "POWER_SUPPLY_STATUS=%s", &charge);
        }
        if(strcmp(&charge, "Charging") == 0)
        {
            for(i=0;i<power_supply_file_num;i++)
            {
                sprintf(temps, "%s/temp", power_supply_dir[i]);
                if(access(temps, W_OK) != 0) continue;
                set_value(temps, "280");
            }
        }
        else
        {
            for(i=0;i<power_supply_file_num;i++)
            {
                sprintf(temps, "%s/temp", power_supply_dir[i]);
                if(access(temps, W_OK) != 0) continue;
                fm = fopen(buffer, "rt");
                fscanf(fm, "%c%c%c", &asdf[0],&asdf[1],&asdf[2]);
                asdf_int = atoi(asdf);
                (asdf_int >= 550)?set_value(temps, "280"):set_value(temps, asdf);
            }
        }
        fclose_file(fe);
        fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
        while(fgets(option, 1000, fc) != NULL)
        {
            sscanf(option, "TEMP_CTRL=%s", &temp_ctrl);
            sscanf(option, "POWER_CTRL=%s", &power_ctrl);
            sscanf(option, "CHARGE_START=%s", &charge_start);
            sscanf(option, "CHARGE_STOP=%s", &charge_stop);
            sscanf(option, "CURRENT_MAX=%s", &current_max);
            sscanf(option, "TEMP_MAX=%s", &temp_max);
            sscanf(option, "HIGHEST_TEMP_CURRENT=%s", &highest_temp_current);
            sscanf(option, "RECHARGE_TEMP=%s", &recharge_temp);
        }
        if(atoi(&power_ctrl) == 1)
        {
            fd = fopen("/sys/class/power_supply/battery/capacity", "rt");
            fgets(&power, 90, fd);
            if(power >= charge_stop)
            {
                if(atoi(&charge_stop) == 100)
                {
                    fm = fopen("/sys/class/power_supply/battery/current_now", "rt");
                    fgets(&done, 15, fm);
                    if(atoi(&done) == 0)
                    {
                        charge_value("0");
                        qwer = 1;
                    }
                    fclose_file(fm);
                }
                else
                {
                    charge_value("0");
                    qwer = 1;
                }
            }
            if(power <= charge_start)
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
        fclose_file(fd);
        if(atoi(&temp_ctrl) == 1)
        {
            fm = fopen(buffer, "rt");
            fgets(thermal, 10, fm);
            temp_int = atoi(thermal);
            sleep(5);
            if(temp_int > atoi(&temp_max)*1000)
            {
                while(temp_int > atoi(&recharge_temp)*1000)
                {
                    fclose_file(fm);
                    fm = fopen(buffer, "rt");
                    fgets(thermal, 300, fm);
                    temp_int = atoi(thermal);
                    sleep(5);
                    fclose_file(fc);
                    fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
                    while(fgets(option, 1000, fc) != NULL)
                    {
                        sscanf(option, "TEMP_CTRL=%s", &temp_ctrl);
                        sscanf(option, "CURRENT_MAX=%s", &current_max);
                        sscanf(option, "TEMP_MAX=%s", &temp_max);
                        sscanf(option, "HIGHEST_TEMP_CURRENT=%s", &highest_temp_current);
                        sscanf(option, "RECHARGE_TEMP=%s", &recharge_temp);
                    }
                    fclose_file(fc);
                    if(atoi(&temp_ctrl) == 0) break;
                    for(i=0;i<power_supply_file_num;i++)
                    {
                        sprintf(constants, "%s/constant_charge_current_max", power_supply_dir[i]);
                        if(access(constants, W_OK) != 0) continue;
                        set_value(constants, &highest_temp_current);
                    }
                }
            }
            for(i=0;i<power_supply_file_num;i++)
            {
                sprintf(constants, "%s/constant_charge_current_max", power_supply_dir[i]);
                if(access(constants, W_OK) != 0) continue;
                set_value(constants, &current_max);
            }
            fclose_file(fm);
            fclose_file(fc);
        }
        else
        {
            sleep(5);
            for(i=0;i<power_supply_file_num;i++)
            {
                sprintf(constants, "%s/constant_charge_current_max", power_supply_dir[i]);
                if(access(constants, W_OK) != 0) continue;
                set_value(constants, &current_max);
            }
            fclose_file(fc);
        }
    }
    return 0;
}
