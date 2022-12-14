#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

void strrpc(char *str, char *oldstr, char *newstr)
{
    char bstr[strlen(str)];
    int i=0;
    memset(bstr,0,sizeof(bstr));
    for(i=0; i<strlen(str);i++)
    {
        if(!strncmp(str+i, oldstr, strlen(oldstr)))
        {
            strcat(bstr,newstr);
            i+=strlen(oldstr)-1;
        }
        else
        {
            strncat(bstr,str+i,1);
        }
    }
    strcpy(str,bstr);
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

void set_value(char *file, char *numb)
{
    FILE *fn;
    if((file != NULL) && (access(file, W_OK) == 0))
    {
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
    char **power_supply_dir,**thermal_dir,done[20],charge[25],power[10],current_max[20],highest_temp_current[20],buffer[100],bms[100]="none",conn_therm[100]="none",battery[100]="none",constants[100],msg[20],thermal[15],option[1010],asdf[15],bat_temp_tmp[1],bat_temp[6];
    int power_supply_file_num,thermal_file_num,i,temp_int,bat_temp_size,asdf_int,fu,qwer,temp_ctrl,power_ctrl,charge_start,charge_stop,recharge_temp,temp_max;
    list_dir("/sys/class/thermal", &thermal_dir, &thermal_file_num);
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
            else if(strcmp(msg, "battery") == 0)
            {
                strrpc(buffer, "type", "temp");
                strcpy(battery, buffer);
            }
            else if(strcmp(msg, "bms") == 0)
            {
                strrpc(buffer, "type", "temp");
                strcpy(bms, buffer);
            }
        }
    }
    strcpy(battery, "/sys/class/power_supply/battery/temp");
    strcpy(bms, "/sys/class/power_supply/bms/temp");
    if(strcmp(conn_therm,"none")==0 || strcmp(battery,"none")==0 || strcmp(bms,"none")==0)
    {
        printf("????????????????????????????????????????????????");
        exit(2);
    }
    else if(access(conn_therm, R_OK) != 0 || access(battery, W_OK) != 0 || access(bms, W_OK) != 0)
    {
        printf("????????????????????????????????????????????????");
        exit(5);
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
            printf("????????????????????????????????????????????????");
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
            printf("??????????????????????????????????????????????????????");
            exit(10);
        }
        fe = fopen("/sys/class/power_supply/battery/status", "rt");
        fgets(charge, 20, fe);
        fclose(fe);
        fe=NULL;
        line_feed(charge);
        if(strcmp(charge, "Charging") == 0)
        {
            if(access(conn_therm, R_OK) != 0 || access(battery, W_OK) != 0 || access(bms, W_OK) != 0)
            {
                printf("????????????????????????????????????????????????");
                exit(20);
            }
            set_value(battery, "280");
            set_value(bms, "280");
            if(power_ctrl == 1)
            {
                if(access("/sys/class/power_supply/battery/capacity", R_OK) != 0)
                {
                    printf("??????????????????????????????????????????????????????");
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
                            printf("??????????????????????????????????????????????????????");
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
                    printf("????????????????????????????????????????????????");
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
                        if(access(conn_therm, R_OK) != 0 || access(battery, W_OK) != 0 || access(bms, W_OK) != 0)
                        {
                            printf("????????????????????????????????????????????????");
                            exit(70);
                        }
                        if(access("/sys/class/power_supply/battery/status", R_OK) != 0)
                        {
                            printf("??????????????????????????????????????????????????????");
                            exit(80);
                        }
                        fe = fopen("/sys/class/power_supply/battery/status", "rt");
                        fgets(charge, 20, fe);
                        fclose(fe);
                        fe=NULL;
                        line_feed(charge);
                        if(strcmp(charge, "Charging") != 0) break;
                        set_value(battery, "280");
                        set_value(bms, "280");
                        if(access(conn_therm, R_OK) != 0)
                        {
                            printf("????????????????????????????????????????????????");
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
                            printf("????????????????????????????????????????????????");
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
                        for(i=0;i<power_supply_file_num;i++)
                        {
                            sprintf(constants, "%s/constant_charge_current_max", power_supply_dir[i]);
                            if(access(constants, W_OK) != 0) continue;
                            set_value(constants, highest_temp_current);
                        }
                        sleep(5);
                    }
                }
            }
            for(i=0;i<power_supply_file_num;i++)
            {
                sprintf(constants, "%s/constant_charge_current_max", power_supply_dir[i]);
                if(access(constants, W_OK) != 0) continue;
                set_value(constants, current_max);
            }
        }
        else
        {
            if(access(conn_therm, R_OK) != 0 || access(battery, W_OK) != 0 || access(bms, W_OK) != 0)
            {
                printf("????????????????????????????????????????????????");
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
            if(strcmp(bat_temp,"000")==0)
            {
                sprintf(bat_temp,"0");
            }
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
                set_value(battery, bat_temp);
                set_value(bms, bat_temp);
            }
            else
            {
                if(asdf_int >= 55000)
                {
                    set_value(battery, "280");
                    set_value(bms, "280");
                }
                else
                {
                    set_value(battery, bat_temp);
                    set_value(bms, bat_temp);
                }
            }
        }
        sleep(5);
    }
    return 0;
}
