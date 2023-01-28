#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "unistd.h"
#include "time.h"
#include "sys/types.h"
#include "sys/stat.h"

struct tm *get_time(int timezone)
{
    unsigned char tmp=0;
    time_t cur_time;
    struct tm *ptm;
    time(&cur_time);
    ptm=gmtime(&cur_time);
    ptm->tm_year+=1900;
    ptm->tm_mon+=1;
    ptm->tm_hour+=timezone;
    if(ptm->tm_hour > 23)
    {
        switch(ptm->tm_mon)
        {
            case 1: case 3: case 5: case 7: case 8: case 10: case 12:
                tmp=1;
                break;
            case 2:
                if(((ptm->tm_year%4 == 0) && (ptm->tm_year%100 != 0)) || (ptm->tm_year%400 == 0)) tmp=3;
                else tmp=2;
                break;
            default:
                tmp=0;
                break;
        }
        ptm->tm_hour-=24;
        ptm->tm_mday+=1;
        if(tmp == 0)
        {
            if(ptm->tm_mday > 30)
            {
                ptm->tm_mday-=30;
                ptm->tm_mon+=1;
            }
        }
        else if(tmp == 1)
        {
            if(ptm->tm_mday > 31)
            {
                ptm->tm_mday-=31;
                ptm->tm_mon+=1;
            }
        }
        else if(tmp == 2)
        {
            if(ptm->tm_mday > 28)
            {
                ptm->tm_mday-=28;
                ptm->tm_mon+=1;
            }
        }
        else if(tmp == 3)
        {
            if(ptm->tm_mday > 29)
            {
                ptm->tm_mday-=29;
                ptm->tm_mon+=1;
            }
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

void check_read_file(char *file)
{
    if(access(file, F_OK) == 0)
    {
        if(access(file, R_OK) != 0)
        {
            chmod(file, 0644);
            if(access(file, R_OK) != 0)
            {
                char chartmp[100];
                snprintf(chartmp,100,"无法读取%s文件，程序强制退出！",file);
                printf_plus_time(chartmp);
                exit(1);
            }
        }
    }
    else
    {
        char chartmp[100];
        snprintf(chartmp,100,"找不到%s文件，程序强制退出！",file);
        printf_plus_time(chartmp);
        exit(999);
    }
}

void powel_ctl(unsigned int opt_new[10], unsigned char tmp[6])
{
    FILE *fd,*fm;
    char chartmp[100],power[10],done[20];
    unsigned char stop=0;
    if(opt_new[2] == 1)
    {
        check_read_file("/sys/class/power_supply/battery/capacity");
        fd = fopen("/sys/class/power_supply/battery/capacity", "rt");
        fgets(power, 5, fd);
        fclose(fd);
        fd=NULL;
        line_feed(power);
        if(atoi(power) >= (int)opt_new[5])
        {
            if(opt_new[5] == 100)
            {
                check_read_file("/sys/class/power_supply/battery/current_now");
                fm = fopen("/sys/class/power_supply/battery/current_now", "rt");
                fgets(done, 15, fm);
                fclose(fm);
                fm=NULL;
                line_feed(done);
                if(atoi(done) == 0)
                {
                    if(!tmp[3])
                    {
                        snprintf(chartmp,100,"当前电量为%s%%，到达停止充电的电量阈值，且输入电流为0A，涓流充电结束，停止充电",power);
                        printf_plus_time(chartmp);
                        tmp[3]=1;
                    }
                    charge_value("0");
                    stop = 1;
                }
            }
            else
            {
                if(!tmp[3])
                {
                    snprintf(chartmp,100,"当前电量为%s%%，到达停止充电的电量阈值，停止充电",power);
                    printf_plus_time(chartmp);
                    tmp[3]=1;
                }
                else
                {
                    if(tmp[5] == 1)
                    {
                        printf_plus_time("新的停止充电的电量阈值高于旧的电量阈值，恢复充电");
                        charge_value("1");
                        stop=0;
                        tmp[3]=0;
                        tmp[5]=0;
                    }
                }
                charge_value("0");
                stop = 1;
            }
        }
        if(atoi(power) <= (int)opt_new[4])
        {
            if(tmp[3])
            {
                snprintf(chartmp,100,"当前电量为%s%%，到达恢复充电的电量阈值，恢复充电",power);
                printf_plus_time(chartmp);
                tmp[3]=0;
            }
            charge_value("1");
            stop = 0;
        }
    }
    else
    {
        if(stop == 1)
        {
            if(tmp[3])
            {
                snprintf(chartmp,100,"当前电量为%s%%，到达恢复充电的电量阈值，恢复充电",power);
                printf_plus_time("电量控制关闭，恢复充电");
                tmp[3]=0;
            }
            charge_value("1");
            stop = 0;
        }
    }
}

int main()
{
    FILE *fq,*fm,*fc,*fd,*fe;
    char options[10][50]={"STEP_CHARGING_DISABLED","TEMP_CTRL","POWER_CTRL","STEP_CHARGING_DISABLED_THRESHOLD","CHARGE_START","CHARGE_STOP","CURRENT_MAX","TEMP_MAX","HIGHEST_TEMP_CURRENT","RECHARGE_TEMP"};
    char **power_supply_dir,**thermal_dir,charge[25],power[10],chartmp[100],current_max_char[20],highest_temp_current_char[20],buffer[100],conn_therm[100]="none",msg[20],thermal[15],option[1010],bat_temp_tmp[1],bat_temp[6];
    int temp_int;
    unsigned char tmp[6]={0,0,0,0,0,0},num=0,fu,i,bat_temp_size,power_supply_file_num,thermal_file_num,opt;
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
    if(strcmp(conn_therm, "none") == 0)
    {
        printf_plus_time("获取温度失败，程序强制退出！");
        exit(2);
    }
    else check_read_file(conn_therm);
    check_read_file("/data/adb/turbo-charge/option.txt");
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
        check_read_file("/data/adb/turbo-charge/option.txt");
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
        fclose(fc);
        fc=NULL;
        if(num>1)
        {
            if(num>10) num=1;
            for(opt=0;opt<10;opt++)
            {
                if(opt_old[opt] != opt_new[opt])
                {
                    snprintf(chartmp,100,"%s值发生改变，新%s值为%d",options[opt],options[opt],opt_new[opt]);
                    printf_plus_time(chartmp);
                    if(opt == 5 && opt_old[5] < opt_new[5]) tmp[5]=1;
                    opt_old[opt]=opt_new[opt];
                }
            }
        }
        else for(opt=0;opt<10;opt++) opt_old[opt]=opt_new[opt];
        snprintf(current_max_char,20,"%u",opt_new[6]);
        snprintf(highest_temp_current_char,20,"%u",opt_new[8]);
        check_read_file("/sys/class/power_supply/battery/status");
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
            if(tmp[1])
            {
                printf_plus_time("充电器已连接");
                tmp[1]=0;
                tmp[2]=1;
            }
            check_read_file(conn_therm);
            list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, "280");
            powel_ctl(opt_new, tmp);
            if(opt_new[1] == 1)
            {
                check_read_file(conn_therm);
                fm = fopen(conn_therm, "rt");
                fgets(thermal, 10, fm);
                fclose(fm);
                fm=NULL;
                line_feed(thermal);
                temp_int = atoi(thermal);
                if(temp_int >= ((int)opt_new[7])*1000)
                {
                    if(!tmp[0])
                    {
                        snprintf(chartmp,100,"温度高于降低充电电流的温度阈值，限制充电电流为%dμA",opt_new[8]);
                        printf_plus_time(chartmp);
                        tmp[0]=1;
                    }
                    while(1)
                    {
                        check_read_file("/data/adb/turbo-charge/option.txt");
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
                        fclose(fc);
                        fc=NULL;
                        for(opt=0;opt<10;opt++)
                        {
                            if(opt_old[opt] != opt_new[opt])
                            {
                                snprintf(chartmp,100,"%s值发生改变，新%s值为%d",options[opt],options[opt],opt_new[opt]);
                                printf_plus_time(chartmp);
                                if(opt == 7 && opt_old[7] < opt_new[7]) tmp[4]=1;
                                opt_old[opt]=opt_new[opt];
                            }
                        }
                        snprintf(current_max_char,20,"%u",opt_new[6]);
                        snprintf(highest_temp_current_char,20,"%u",opt_new[8]);
                        if(tmp[4] == 1)
                        {
                            snprintf(chartmp,100,"新的降低充电电流的温度阈值高于旧的温度阈值，恢复充电电流为%dμA",opt_new[6]);
                            printf_plus_time(chartmp);
                            tmp[4]=0;
                            break;
                        }
                        check_read_file("/sys/class/power_supply/battery/status");
                        fe = fopen("/sys/class/power_supply/battery/status", "rt");
                        fgets(charge, 20, fe);
                        fclose(fe);
                        fe=NULL;
                        line_feed(charge);
                        if(strcmp(charge, "Charging") != 0 && strcmp(charge, "Full") != 0)
                        {
                            snprintf(chartmp,100,"充电器断开连接，恢复充电电流为%dμA",opt_new[6]);
                            printf_plus_time(chartmp);
                            tmp[1]=1;
                            tmp[2]=0;
                            break;
                        }
                        check_read_file(conn_therm);
                        fm = fopen(conn_therm, "rt");
                        fgets(thermal, 300, fm);
                        fclose(fm);
                        fm=NULL;
                        line_feed(thermal);
                        temp_int = atoi(thermal);
                        if(temp_int <= ((int)opt_new[9])*1000)
                        {
                            snprintf(chartmp,100,"温度低于恢复快充的温度阈值，恢复充电电流为%dμA",opt_new[6]);
                            printf_plus_time(chartmp);
                            break;
                        }
                        if(opt_new[1] == 0)
                        {
                            snprintf(chartmp,100,"温控关闭，恢复充电电流为%dμA",opt_new[6]);
                            printf_plus_time(chartmp);
                            break;
                        }
                        if(opt_new[0] == 1) (atoi(power) <= (int)opt_new[3])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
                        else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
                        list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, "280");
                        list_dir_set_value(power_supply_dir, "constant_charge_current_max", power_supply_file_num, highest_temp_current_char);
                        powel_ctl(opt_new, tmp);
                        sleep(5);
                    }
                    tmp[0]=0;
                }
            }
            list_dir_set_value(power_supply_dir, "constant_charge_current_max", power_supply_file_num, current_max_char);
        }
        else
        {
            if(!tmp[2] && !tmp[1])
            {
                printf_plus_time("充电器未连接");
                tmp[1]=1;
            }
            else if(!tmp[1])
            {
                printf_plus_time("充电器断开连接");
                tmp[1]=1;
            }
            if(opt_new[0] == 1) (atoi(power) <= (int)opt_new[3])?set_value("/sys/class/power_supply/battery/step_charging_enabled", "1"):set_value("/sys/class/power_supply/battery/step_charging_enabled", "0");
            else set_value("/sys/class/power_supply/battery/step_charging_enabled", "1");
            check_read_file(conn_therm);
            fm = fopen(conn_therm, "rt");
            fgets(thermal, 10, fm);
            fclose(fm);
            fm=NULL;
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
                list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, bat_temp);
            }
            else (temp_int >= 55000)?list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, "280"):list_dir_set_value(power_supply_dir, "temp", power_supply_file_num, bat_temp);
        }
        sleep(5);
    }
    return 0;
}
