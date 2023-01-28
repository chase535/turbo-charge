#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "unistd.h"
#include "time.h"
#include "sys/types.h"
#include "sys/stat.h"

void printf_plus_time(char *dat)
{
    time_t cur_time;
    struct tm *ptm;
    time(&cur_time);
    ptm=localtime(&cur_time);
    printf ("[ %04d.%02d.%02d %02d:%02d:%02d ] %s\n", (ptm->tm_year)+1900, (ptm->tm_mon)+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, dat);
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
    char childpath[600];
    *ppp=(char**)malloc(sizeof(char *) * 1000);
    pDir = opendir(path);
    if(pDir != NULL)
    {
        while ((ent = readdir(pDir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            sprintf(childpath,"%s/%s",path,ent->d_name);
            (*ppp)[file_num]=(char *)malloc(sizeof(char) * (strlen(childpath) + 1));
            strcpy((*ppp)[file_num],childpath);
            file_num++;
        }
        closedir(pDir);
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
    char chartmp[100];
    if(access(file, F_OK) != 0)
    {
        snprintf(chartmp,100,"无法找到%s文件，程序强制退出！", file);
        printf_plus_time(chartmp);
        exit(999);
    }
}

int list_dir_check_file(char *file_dir, char *file_name)
{
    int i,j=0,k,file_num;
    char file[100], **dir;
    file_num=list_dir(file_dir, &dir);
    for(i=0;i<file_num;i++)
    {
        sprintf(file, "%s/%s", dir[i], file_name);
        if(access(file, F_OK) == 0) j++;
    }
    k=(j == 0)?0:1;
    return k;
}

int main()
{
    FILE *fq,*fm,*fc,*fd,*fe;
    char options[10][50]={"STEP_CHARGING_DISABLED","TEMP_CTRL","POWER_CTRL","STEP_CHARGING_DISABLED_THRESHOLD","CHARGE_START","CHARGE_STOP","CURRENT_MAX","TEMP_MAX","HIGHEST_TEMP_CURRENT","RECHARGE_TEMP"};
    char **power_supply_dir,**thermal_dir,done[20],charge[25],power[10],chartmp[100],current_max_char[20],highest_temp_current_char[20],buffer[100],conn_therm[100]="none",msg[20],thermal[15],option[1010],asdf[15],bat_temp_tmp[1],bat_temp[6];
    int opt,power_supply_file_num,thermal_file_num,temp_int,bat_temp_size,asdf_int,i,fu,qwer=0,num=0;
    unsigned int opt_old[10]={0,0,0,0,0,0,0,0,0,0},opt_new[10]={0,0,0,0,0,0,0,0,0,0};
    check_file("/sys/class/power_supply/battery/status");
    check_file("/sys/class/power_supply/battery/current_now");
    check_file("/sys/class/power_supply/battery/capacity");
    if(access("/sys/class/power_supply/battery/step_charging_enabled", F_OK) != 0) printf_plus_time("由于找不到/sys/class/power_supply/battery/step_charging_enabled文件，阶梯充电有关功能失效！");
    if(list_dir_check_file("/sys/class/power_supply", "constant_charge_current_max") == 0) printf_plus_time("无法在/sys/class/power_supply中的所有文件夹内找到constant_charge_current_max文件，电流调节有关功能失效！");
    if(list_dir_check_file("/sys/class/power_supply", "temp") == 0) printf_plus_time("无法在/sys/class/power_supply中的所有文件夹内找到temp文件，充电时强制显示28℃功能失效！");
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
        printf_plus_time("获取温度失败，程序强制退出！");
        exit(2);
    }
    if(access("/data/adb/turbo-charge/option.txt", R_OK) != 0)
    {
        printf_plus_time("配置文件丢失，程序强制退出！");
        exit(1);
    }
    printf_plus_time("文件检测完毕，程序开始运行");
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
        num++;
        if(access("/data/adb/turbo-charge/option.txt", R_OK) != 0)
        {
            printf_plus_time("配置文件丢失，程序强制退出！");
            exit(1);
        }
        fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
        while(fgets(option, 1000, fc) != NULL)
        {
            sscanf(option, "STEP_CHARGING_DISABLED=%u", &opt_new[0]);
            sscanf(option, "TEMP_CTRL=%u", &opt_new[1]);
            sscanf(option, "POWER_CTRL=%u", &opt_new[2]);
            sscanf(option, "STEP_CHARGING_DISABLED_THRESHOLD=%u", &opt_new[3]);
            sscanf(option, "CHARGE_START=%u", &opt_new[4]);
            sscanf(option, "CHARGE_STOP=%u", &opt_new[5]);
            sscanf(option, "CURRENT_MAX=%u", &opt_new[6]);
            sscanf(option, "TEMP_MAX=%u", &opt_new[7]);
            sscanf(option, "HIGHEST_TEMP_CURRENT=%u", &opt_new[8]);
            sscanf(option, "RECHARGE_TEMP=%u", &opt_new[9]);
        }
        if(num>1)
        {
            if(num>10) num=1;
            for(opt=0;opt<10;opt++)
            {
                if(opt_old[opt] != opt_new[opt])
                {
                    snprintf(chartmp,100,"%s值发生改变，新%s值为%d",options[opt],options[opt],opt_new[opt]);
                    printf_plus_time(chartmp);
                    opt_old[opt]=opt_new[opt];
                }
            }
        }
        else for(opt=0;opt<10;opt++) opt_old[opt]=opt_new[opt];
        snprintf(current_max_char,20,"%u",opt_new[6]);
        snprintf(highest_temp_current_char,20,"%u",opt_new[8]);
        fclose(fc);
        fc=NULL;
        if(access("/sys/class/power_supply/battery/status", R_OK) != 0)
        {
            printf_plus_time("读取充电状态失败，程序强制退出！");
            exit(10);
        }
        fd = fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fd);
        fclose(fd);
        fd=NULL;
        line_feed(power);
        if(opt_new[0] == 1) (atoi(power) <= (int)opt_new[3])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
        else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
        fe = fopen("/sys/class/power_supply/battery/status", "rt");
        fgets(charge, 20, fe);
        fclose(fe);
        fe=NULL;
        line_feed(charge);
        if(strcmp(charge, "Charging") == 0 || strcmp(charge, "Full") == 0)
        {
            if(access(conn_therm, R_OK) != 0)
            {
                printf_plus_time("获取温度失败，程序强制退出！");
                exit(220);
            }
            list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, "280");
            if(opt_new[2] == 1)
            {
                if(access("/sys/class/power_supply/battery/capacity", R_OK) != 0)
                {
                    printf_plus_time("获取电量信息失败，程序强制退出！");
                    exit(40);
                }
                fd = fopen("/sys/class/power_supply/battery/capacity", "rt");
                fgets(power, 5, fd);
                fclose(fd);
                fd=NULL;
                line_feed(power);
                if(atoi(power) >= (int)opt_new[5])
                {
                    if(opt_new[5] == 100)
                    {
                        if(access("/sys/class/power_supply/battery/current_now", R_OK) != 0)
                        {
                            printf_plus_time("获取电流信息失败，程序强制退出！");
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
                else if(atoi(power) <= (int)opt_new[4])
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
            if(opt_new[1] == 1)
            {
                if(access(conn_therm, R_OK) != 0)
                {
                    printf_plus_time("获取温度失败，程序强制退出！");
                    exit(60);
                }
                fm = fopen(conn_therm, "rt");
                fgets(thermal, 10, fm);
                fclose(fm);
                fm=NULL;
                line_feed(thermal);
                temp_int = atoi(thermal);
                if(temp_int > ((int)opt_new[7])*1000)
                {
                    while(temp_int > ((int)opt_new[9])*1000)
                    {
                        if(access(conn_therm, R_OK) != 0)
                        {
                            printf_plus_time("获取温度失败，程序强制退出！");
                            exit(70);
                        }
                        if(access("/sys/class/power_supply/battery/status", R_OK) != 0)
                        {
                            printf_plus_time("读取充电状态失败，程序强制退出！");
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
                            printf_plus_time("获取温度失败，程序强制退出！");
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
                            printf_plus_time("配置文件丢失，程序强制退出！");
                            exit(100);
                        }
                        fc = fopen("/data/adb/turbo-charge/option.txt", "rt");
                        while(fgets(option, 1000, fc) != NULL)
                        {
                            sscanf(option, "STEP_CHARGING_DISABLED=%u", &opt_new[0]);
                            sscanf(option, "TEMP_CTRL=%u", &opt_new[1]);
                            sscanf(option, "POWER_CTRL=%u", &opt_new[2]);
                            sscanf(option, "STEP_CHARGING_DISABLED_THRESHOLD=%u", &opt_new[3]);
                            sscanf(option, "CHARGE_START=%u", &opt_new[4]);
                            sscanf(option, "CHARGE_STOP=%u", &opt_new[5]);
                            sscanf(option, "CURRENT_MAX=%u", &opt_new[6]);
                            sscanf(option, "TEMP_MAX=%u", &opt_new[7]);
                            sscanf(option, "HIGHEST_TEMP_CURRENT=%u", &opt_new[8]);
                            sscanf(option, "RECHARGE_TEMP=%u", &opt_new[9]);
                        }
                        for(opt=0;opt<10;opt++)
                        {
                            if(opt_old[opt] != opt_new[opt])
                            {
                                snprintf(chartmp,100,"%s值发生改变，新%s值为%d",options[opt],options[opt],opt_new[opt]);
                                printf_plus_time(chartmp);
                                opt_old[opt]=opt_new[opt];
                            }
                        }
                        snprintf(current_max_char,20,"%u",opt_new[6]);
                        snprintf(highest_temp_current_char,20,"%u",opt_new[8]);
                        fclose(fc);
                        fc=NULL;
                        if(opt_new[1] == 0) break;
                        if(opt_new[0] == 1) (atoi(power) <= (int)opt_new[3])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
                        else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
                        list_dir_set_value(power_supply_dir, "constant_charge_current_max", power_supply_file_num, highest_temp_current_char);
                        sleep(5);
                    }
                }
            }
            list_dir_set_value(power_supply_dir, "constant_charge_current_max", power_supply_file_num, current_max_char);
        }
        else
        {
            if(opt_new[0] == 1) (atoi(power) <= (int)opt_new[3])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
            else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
            if(access(conn_therm, R_OK) != 0)
            {
                printf_plus_time("获取温度失败，程序强制退出！");
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
        sleep(5);
    }
    return 0;
}
