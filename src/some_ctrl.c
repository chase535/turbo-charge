#include "some_ctrl.h"

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