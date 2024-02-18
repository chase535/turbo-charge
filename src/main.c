#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <regex.h>
#include <time.h>
#include <sys/stat.h>

#include "main.h"
#include "read_option.h"
#include "some_ctrl.h"
#include "my_malloc.h"
#include "printf_with_time.h"
#include "value_set.h"
#include "foreground_app.h"

/*
列出path目录下的所有文件夹并赋值给二级指针变量ppp
第二个参数是二级指针的地址
返回值是文件夹的个数
*/
int list_dir(char *path, char ***ppp)
{
    DIR *pDir;
    struct dirent *ent;
    int file_num=0;
    pDir=opendir(path);
    if(pDir != NULL)
    {
        //重新分配一个足够大的内存，用来记录文件夹的个数
        *ppp=(char **)my_realloc(*ppp, sizeof(char *)*500);
        while((ent=readdir(pDir)) != NULL)
        {
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
            //分配一个与路径字符串大小相等的内存，用来装路径
            (*ppp)[file_num]=(char *)my_calloc(1, sizeof(char)*((strlen(path)+strlen(ent->d_name))+2));
            //拼接路径及文件夹名
            sprintf((*ppp)[file_num], "%s/%s", path, ent->d_name);
            file_num++;
        }
        closedir(pDir);
        //收缩内存
        *ppp=(char **)my_realloc(*ppp, sizeof(char *)*file_num);
    }
    return file_num;
}

//将字符串中的回车符、换行符换为字符串结束符
void line_feed(char *line)
{
    char *p;
    p=strchr(line, '\r');
    if(p != NULL) *p='\0';
    p=strchr(line, '\n');
    if(p != NULL) *p='\0';
}

/*
检查文件是否可读
若文件存在且不可读，延时0.25秒后重新检查
检查两次是为了兼容在本程序检查时刚好被系统的其他进程锁住（如刚好进行了文件的保存操作）导致的无法读取
若仍不可读尝试修改权限后重新检查是否可读
若文件不存在或修改权限后仍不可读，则强制停止程序运行
*/
void check_read_file(char *file)
{
    if(!access(file, F_OK))
    {
        check_permission:
        if(access(file, R_OK))
        {
            //权限0644不得简写成644
            chmod(file, 0644);
            if(access(file, R_OK))
            {
                printf_with_time("无法读取%s文件，程序强制退出！", file);
                exit(1);
            }
        }
    }
    else
    {
        struct timespec req={0, 250000000L};
        nanosleep(&req, NULL);
        if(!access(file, F_OK)) goto check_permission;
        else
        {
            printf_with_time("找不到%s文件，程序强制退出！", file);
            exit(999);
        }
    }
}

//读取文件内容，若文件不存在则程序强制退出
void read_file(char *file_path, char *char_var, int max_char_num)
{
    FILE *fp;
    check_read_file(file_path);
    fp=fopen(file_path, "rt");
    fgets(char_var, max_char_num, fp);
    fclose(fp);
    fp=NULL;
    line_feed(char_var);
}

int main()
{
    FILE *fq;
    char **current_limit_file,**power_supply_dir_list,**power_supply_dir,**thermal_dir,**current_max_file,**temp_file,charge[25],power[10];
    char *temp_tmp,*temp_sensor,*temp_sensor_dir,*buffer,*msg,current_max_char[20],highest_temp_current_char[20],thermal[15],last_appname[APP_PACKAGE_NAME_MAX_SIZE];
    uchar step_charge=1,power_control=1,force_temp=1,has_force_temp=0,current_change=1,battery_status=1,battery_capacity=1;
    int i=0,j=0,temp_sensor_num=100,temp_int=0,power_supply_file_num=0,thermal_file_num=0,current_limit_file_num=0,last_charge_stop=-1,charge_is_stop=0,is_first_time=1;
    int power_supply_dir_list_num=0,current_max_file_num=0,temp_file_num=0,is_bypass=0,can_get_foreground=0,screen_is_off=0,last_temp_max=-1,last_charge_status=0;
    regex_t temp_re,current_max_re,current_limit_re;
    regmatch_t temp_pmatch,current_max_pmatch,current_limit_pmatch;
    pthread_t thread1,thread2;
    struct stat statbuf;
    //初始化链表
    options_linkedlist_init();
    printf("作者：酷安@诺鸡鸭\n");
    printf("GitHub开源地址：https://github.com/chase535/turbo-charge\n\n");
    //如果是写入文件，则必须加上这句话，不然只能等缓冲区满了后才会一次性写入
    fflush(stdout);
    //判断各个所需文件是否存在
    if(access("/sys/class/power_supply/battery/status", F_OK)) battery_status=0;
    if(access("/sys/class/power_supply/battery/capacity", F_OK)) battery_capacity=0;
    if(!battery_status || !battery_capacity)
    {
        power_control=0;
        if(battery_status && !battery_capacity) printf_with_time("由于找不到/sys/class/power_supply/battery/capacity文件，电量控制功能失效！");
        else if(!battery_status && battery_capacity) printf_with_time("由于找不到/sys/class/power_supply/battery/status文件，电量控制功能失效，且“伪”旁路供电功能无法根据手机的充电状态而自动启停！");
        else printf_with_time("由于找不到/sys/class/power_supply/battery/status和/sys/class/power_supply/battery/capacity文件，电量控制功能失效，且“伪”旁路供电功能无法根据手机的充电状态而自动启停！");
    }
    else if(access("/sys/class/power_supply/battery/charging_enabled", F_OK) && access("/sys/class/power_supply/battery/battery_charging_enabled", F_OK) && access("/sys/class/power_supply/battery/input_suspend", F_OK) && access("/sys/class/qcom-battery/restricted_charging", F_OK))
    {
        power_control=0;
        printf_with_time("由于找不到控制手机暂停充电的文件，电量控制功能失效！");
        printf_with_time("目前已知的有关文件有：/sys/class/power_supply/battery/charging_enabled、/sys/class/power_supply/battery/battery_charging_enabled、/sys/class/power_supply/battery/input_suspend、/sys/class/qcom-battery/restricted_charging");
        printf_with_time("如果您知道其他的有关文件，请联系模块制作者！");
    }
    if(access("/sys/class/power_supply/battery/step_charging_enabled", F_OK))
    {
        step_charge=0;
        printf_with_time("由于找不到/sys/class/power_supply/battery/step_charging_enabled文件，阶梯式充电控制的所有功能失效！");
    }
    else if(!battery_capacity)
    {
        step_charge=2;
        printf_with_time("由于找不到/sys/class/power_supply/battery/capacity文件，阶梯式充电无法根据电量进行开关，此时若在配置中关闭阶梯式充电，则无论电量多少，阶梯式充电都会关闭！");
    }
    //使用正则表达式，用来模糊查找相应文件
    regcomp(&current_max_re, ".*/constant_charge_current_max$|.*/fast_charge_current$|.*/thermal_input_current$", REG_EXTENDED|REG_NOSUB);
    regcomp(&current_limit_re, ".*/thermal_input_current_limit$", REG_EXTENDED|REG_NOSUB);
    regcomp(&temp_re, ".*/temp$", REG_EXTENDED|REG_NOSUB);
    power_supply_dir=(char **)my_calloc(1, sizeof(char *));
    power_supply_file_num=list_dir("/sys/class/power_supply", &power_supply_dir);
    //预先分配一个足够大的内存，用来记录文件的个数
    current_limit_file=(char **)my_calloc(1, sizeof(char *)*100);
    current_max_file=(char **)my_calloc(1, sizeof(char *)*100);
    temp_file=(char **)my_calloc(1, sizeof(char *)*100);
    //遍历/sys/class/power_supply
    for(i=0;i < power_supply_file_num;i++)
    {
        power_supply_dir_list=(char **)my_calloc(1, sizeof(char *));
        power_supply_dir_list_num=list_dir(power_supply_dir[i], &power_supply_dir_list);
        //遍历/sys/class/power_supply路径下的所有文件夹
        for(j=0;j < power_supply_dir_list_num;j++)
        {
            //如果匹配到了文件，则分配一个与路径字符串大小相等的内存，装入路径，并且记录相应文件数量的变量值+1
            if(!regexec(&current_limit_re, power_supply_dir_list[j], 1, &current_limit_pmatch, 0))
            {
                current_limit_file[current_limit_file_num]=(char *)my_calloc(1, sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(current_limit_file[current_limit_file_num], power_supply_dir_list[j]);
                current_limit_file_num++;
            }
            if(!regexec(&current_max_re, power_supply_dir_list[j], 1, &current_max_pmatch, 0))
            {
                current_max_file[current_max_file_num]=(char *)my_calloc(1, sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(current_max_file[current_max_file_num], power_supply_dir_list[j]);
                current_max_file_num++;
            }
            if(!regexec(&temp_re, power_supply_dir_list[j], 1, &temp_pmatch, 0))
            {
                temp_file[temp_file_num]=(char *)my_calloc(1, sizeof(char)*(strlen(power_supply_dir_list[j])+1));
                strcpy(temp_file[temp_file_num], power_supply_dir_list[j]);
                temp_file_num++;
            }
        }
        free_malloc_memory(&power_supply_dir_list, power_supply_dir_list_num);
    }
    free_malloc_memory(&power_supply_dir, power_supply_file_num);
    //收缩内存
    current_limit_file=(char **)my_realloc(current_limit_file, sizeof(char *)*current_limit_file_num);
    current_max_file=(char **)my_realloc(current_max_file, sizeof(char *)*current_max_file_num);
    temp_file=(char **)my_realloc(temp_file, sizeof(char *)*temp_file_num);
    //判断文件数量是否为0
    if(!current_max_file_num)
    {
        current_change=0;
        printf_with_time("无法在/sys/class/power_supply中的所有文件夹内找到constant_charge_current_max、fast_charge_current、thermal_input_current文件，有关电流的所有功能（包括“伪”旁路供电功能）失效！");
    }
    if(!battery_status || !temp_file_num)
    {
        force_temp=0;
        if(battery_status && !temp_file_num) printf_with_time("无法在/sys/class/power_supply中的所有文件夹内找到temp文件，充电时强制显示28℃功能失效！");
        else if(!battery_status && temp_file_num) printf_with_time("由于找不到/sys/class/power_supply/battery/status文件，充电时强制显示28℃功能失效！");
        else printf_with_time("由于找不到/sys/class/power_supply/battery/status文件以及无法在/sys/class/power_supply中的所有文件夹内找到temp文件，充电时强制显示28℃功能失效！");
    }
    //预先分配一个占位用的内存，此内存后续将装入温度传感器所获取到的温度的文件的路径
    temp_sensor=(char *)my_calloc(1, sizeof(char));
    if(force_temp || current_change)
    {
        //预先分配一个占位用的内存
        temp_sensor_dir=(char *)my_calloc(1, sizeof(char));
        buffer=(char *)my_calloc(1, sizeof(char));
        temp_tmp=(char *)my_calloc(1, sizeof(char)*15);
        msg=(char *)my_calloc(1, sizeof(char));
        thermal_dir=(char **)my_calloc(1, sizeof(char *));
        thermal_file_num=list_dir("/sys/class/thermal", &thermal_dir);
        //遍历/sys/class/thermal
        for(i=0;i < thermal_file_num;i++)
        {
            //判断/sys/class/thermal下各个文件夹的名是否包含thermal_zone
            //因为温度传感器的相关文件就在此类文件夹中
            if(strstr(thermal_dir[i], "thermal_zone") != NULL)
            {
                //判断该文件夹内是否有一个名为type的文件，此文件的内容为该温度传感器的名称
                buffer=(char *)my_realloc(buffer, sizeof(char)*(strlen(thermal_dir[i])+6));
                sprintf(buffer, "%s/type", thermal_dir[i]);
                //如果没有type文件或type文件不可读，则跳过此温度传感器的后续操作
                if(access(buffer, R_OK)) continue;
                //获取type文件内的字符个数
                stat(buffer, &statbuf);
                fq=fopen(buffer, "rt");
                if(fq != NULL)
                {
                    //重新分配内存，使其刚好能够装入该温度传感器的名称，并获取该温度传感器的名称
                    msg=(char *)my_realloc(msg, sizeof(char)*(statbuf.st_size+1));
                    fgets(msg, statbuf.st_size+1, fq);
                    fclose(fq);
                    fq=NULL;
                }
                //如果无法通过管道打开type文件，则跳过此温度传感器的后续操作
                else continue;
                line_feed(msg);
                //检查此文件夹下有无temp文件，此文件的内容是该温度传感器所获取的温度值
                sprintf(buffer, "%s/temp", thermal_dir[i]);
                //如果没有temp文件或temp文件不可读，则跳过此温度传感器的后续操作
                if(access(buffer, R_OK)) continue;
                fq=fopen(buffer, "rt");
                if(fq != NULL)
                {
                    fgets(temp_tmp, 10, fq);
                    fclose(fq);
                    fq=NULL;
                }
                //如果无法通过管道打开temp文件，则跳过此温度传感器的后续操作
                else continue;
                line_feed(temp_tmp);
                //判断temp文件的值是否正常，若不正常则代表不能使用此温度传感器，跳过此温度传感器的后续操作
                if(atoi(temp_tmp) == 1 || atoi(temp_tmp) == 0 || atoi(temp_tmp) == -1) continue;
                //根据温度传感器的优先级顺序进行筛选，最终选择优先级最高的可用的温度传感器
                for(j=0;j < temp_sensor_quantity;j++)
                {
                    if(!strcmp(msg, temp_sensors[j]) && temp_sensor_num > j)
                    {
                        temp_sensor_num=j;
                        //重新分配内存保证了最小内存使用量，temp_sensor_dir只是温度传感器的所在路径，后续会进行拼接
                        temp_sensor_dir=(char *)my_realloc(temp_sensor_dir, sizeof(char)*(strlen(thermal_dir[i])+1));
                        strcpy(temp_sensor_dir, thermal_dir[i]);
                    }
                }
            }
        }
        my_free(buffer);
        buffer=NULL;
        my_free(temp_tmp);
        temp_tmp=NULL;
        my_free(msg);
        msg=NULL;
        free_malloc_memory(&thermal_dir, thermal_file_num);
        //判断是否获取到了可用的温度传感器
        if(temp_sensor_num != 100)
        {
            //重新分配内存，使其刚好能够装下路径+“/temp”，因为temp文件内存储的是温度传感器所获取的温度值
            temp_sensor=(char *)my_realloc(temp_sensor, sizeof(char)*(strlen(temp_sensor_dir)+6));
            sprintf(temp_sensor, "%s/temp", temp_sensor_dir);
            printf_with_time("将使用%s温度传感器作为手机温度的获取源。由于每个传感器所处位置不同以及每个手机发热区不同，很可能导致获取到的温度与实际体感温度不同", temp_sensors[temp_sensor_num]);
            check_read_file(temp_sensor);
        }
        //如果没有获取到可用的温度传感器，则打印相关信息
        else
        {
            my_free(temp_sensor);
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
        my_free(temp_sensor_dir);
        temp_sensor_dir=NULL;
    }
    else if(!step_charge && !power_control && !force_temp && !current_change)
    {
        printf_with_time("所有的所需文件均不存在，完全不适配此手机，程序强制退出！");
        exit(1000);
    }
    if(current_change) for(i=0;i < current_max_file_num;i++) printf_with_time("找到电流文件：%s", current_max_file[i]);
    if(force_temp) for(i=0;i < temp_file_num;i++) printf_with_time("找到温度文件：%s", temp_file[i]);
    //如果有电流文件，则获取安卓版本，为以后“伪”旁路供电做准备
    if(current_change) can_get_foreground=check_android_version();
    //创建一个子线程用来读取配置文件
    pthread_create(&thread2, NULL, read_options, NULL);
    pthread_detach(thread2);
    snprintf(current_max_char, 20, "%d", read_one_option("CURRENT_MAX"));
    snprintf(highest_temp_current_char, 20, "%d", read_one_option("HIGHEST_TEMP_CURRENT"));
    printf_with_time("文件检测完毕，程序开始运行");
    //写入初值
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
    set_array_value(current_limit_file, current_limit_file_num, "-1");
    charge_ctl("1");
    //前期工作全部完成，程序正式开始运行
    while(1)
    {
        //读配置文件并赋值给程序内部变量
        snprintf(current_max_char, 20, "%d", read_one_option("CURRENT_MAX"));
        snprintf(highest_temp_current_char, 20, "%d", read_one_option("HIGHEST_TEMP_CURRENT"));
        //如果无法判断手机的充电状态，则很多功能无法实现，剩余代码很简单
        if(battery_status)
        {
            //阶梯充电
            if(step_charge == 1)
            {
                if(read_one_option("STEP_CHARGING_DISABLED") == 1) (atoi(power) < read_one_option("STEP_CHARGING_DISABLED_THRESHOLD"))?step_charge_ctl("1"):step_charge_ctl("0");
                else step_charge_ctl("1");
            }
            else if(step_charge == 2) (read_one_option("STEP_CHARGING_DISABLED") == 1)?step_charge_ctl("0"):step_charge_ctl("1");
            //“伪”旁路供电
            bypass_charge_ctl(&thread1, &can_get_foreground, last_appname, &is_bypass, &screen_is_off, current_max_file, current_max_file_num);
            if(!screen_is_off)
            {
                sleep(read_one_option("CYCLE_TIME"));
                continue;
            }
            //向电流文件写入配置文件中所设置的最大电流
            if(current_change) set_array_value(current_max_file, current_max_file_num, current_max_char);
            sleep(read_one_option("CYCLE_TIME"));
            continue;
        }
        if(force_temp && read_one_option("FORCE_TEMP") == 1 && !has_force_temp) has_force_temp=1;
        //读取手机电量
        read_file("/sys/class/power_supply/battery/capacity", power, sizeof(power));
        //阶梯充电，无论是否在充电，都会跟随配置文件进行更改
        if(step_charge == 1)
        {
            if(read_one_option("STEP_CHARGING_DISABLED") == 1) (atoi(power) < read_one_option("STEP_CHARGING_DISABLED_THRESHOLD"))?step_charge_ctl("1"):step_charge_ctl("0");
            else step_charge_ctl("1");
        }
        else if(step_charge == 2) (read_one_option("STEP_CHARGING_DISABLED") == 1)?step_charge_ctl("0"):step_charge_ctl("1");
        //读取充电状态
        read_file("/sys/class/power_supply/battery/status", charge, sizeof(charge));
        //如果手机在充电，则根据配置文件进行相应操作
        if(strcmp(charge, "Discharging"))
        {
            //此处通过两个变量来确定上次循环时的充电状态以及是否是程序的第一次循环
            if(is_first_time || !last_charge_status)
            {
                printf_with_time("充电器已连接");
                last_charge_status=1;
                is_first_time=0;
            }
            //充电时强制显示28度功能
            if(force_temp && read_one_option("FORCE_TEMP") == 1) set_array_value(temp_file, temp_file_num, "280");
            else if(has_force_temp) set_temp(temp_sensor, temp_file, temp_file_num, 0);
            //电量控制功能
            if(power_control) powel_ctl(&last_charge_stop, &charge_is_stop);
            //如果电流文件存在且安卓版本大于等于7，则执行“伪”旁路供电函数
            if(current_change && can_get_foreground)
            {
                bypass_charge_ctl(&thread1, &can_get_foreground, last_appname, &is_bypass, &screen_is_off, current_max_file, current_max_file_num);
                //如果进入了“伪”旁路供电模式，则不执行下面的温度控制相关函数，直接进入下次循环
                if(is_bypass && !screen_is_off)
                {
                    sleep(read_one_option("CYCLE_TIME"));
                    continue;
                }
            }
            //温度控制功能
            if(read_one_option("TEMP_CTRL") == 1 && temp_sensor_num != 100 && current_change)
            {
                //读取温度传感器所获取的温度值
                read_file(temp_sensor, thermal, sizeof(thermal));
                temp_int=atoi(thermal);
                //如果温度大于等于配置文件所设置的值，则打印相关信息并进入另一个循环中，等待相关事件的发生后退出二层循环
                if(temp_int >= read_one_option("TEMP_MAX")*1000)
                {
                    printf_with_time("手机温度大于等于降低充电电流的温度阈值，限制充电电流为%sμA", highest_temp_current_char);
                    //如果进入了“伪”旁路供电模式则退出此循环
                    while(!is_bypass)
                    {
                        //每次循环重新读取配置文件及温度值
                        snprintf(current_max_char, 20, "%d", read_one_option("CURRENT_MAX"));
                        snprintf(highest_temp_current_char, 20, "%d", read_one_option("HIGHEST_TEMP_CURRENT"));
                        set_array_value(current_limit_file, current_limit_file_num, "-1");
                        if(force_temp && read_one_option("FORCE_TEMP") == 1 && !has_force_temp) has_force_temp=1;
                        read_file(temp_sensor, thermal, sizeof(thermal));
                        temp_int=atoi(thermal);
                        //此tmp数组在读取配置文件的函数中，若配置文件的TEMP_MAX发生改变则会得到相应更改
                        //作用是判断新的降低充电电流的温度阈值与旧的温度阈值的大小
                        if(last_temp_max == -1) last_temp_max=read_one_option("TEMP_MAX");
                        if(last_temp_max != read_one_option("TEMP_MAX"))
                        {
                            last_temp_max=read_one_option("TEMP_MAX");
                            if(temp_int < read_one_option("TEMP_MAX")*1000)
                            {
                                printf_with_time("新的降低充电电流的温度阈值高于旧的温度阈值，且手机温度小于新的温度阈值，恢复充电电流为%sμA", current_max_char);
                                break;
                            }
                            else printf_with_time("新的降低充电电流的温度阈值高于旧的温度阈值，但手机温度大于等于新的温度阈值，限制充电电流为%sμA", highest_temp_current_char);
                            last_temp_max=read_one_option("TEMP_MAX");
                        }
                        //判断手机充电状态
                        read_file("/sys/class/power_supply/battery/status", charge, sizeof(charge));
                        if(!strcmp(charge, "Discharging"))
                        {
                            printf_with_time("充电器断开连接，恢复充电电流为%sμA", current_max_char);
                            last_charge_status=0;
                            break;
                        }
                        //判断温度是否小于恢复快充的温度阈值
                        if(temp_int <= read_one_option("RECHARGE_TEMP")*1000)
                        {
                            printf_with_time("手机温度小于等于恢复快充的温度阈值，恢复充电电流为%sμA", current_max_char);
                            break;
                        }
                        //判断温度控制功能是否关闭
                        if(!read_one_option("TEMP_CTRL"))
                        {
                            printf_with_time("温控关闭，恢复充电电流为%sμA", current_max_char);
                            break;
                        }
                        //阶梯充电
                        if(step_charge == 1)
                        {
                            if(read_one_option("STEP_CHARGING_DISABLED") == 1) (atoi(power) < read_one_option("STEP_CHARGING_DISABLED_THRESHOLD"))?step_charge_ctl("1"):step_charge_ctl("0");
                            else step_charge_ctl("1");
                        }
                        else if(step_charge == 2) (read_one_option("STEP_CHARGING_DISABLED") == 1)?step_charge_ctl("0"):step_charge_ctl("1");
                        //向电流文件中写入限制电流数值
                        set_array_value(current_max_file, current_max_file_num, highest_temp_current_char);
                        //充电时强制显示28度功能
                        if(force_temp && read_one_option("FORCE_TEMP") == 1) set_array_value(temp_file, temp_file_num, "280");
                        else if(has_force_temp) set_temp(temp_sensor, temp_file, temp_file_num, 0);
                        //电量控制功能
                        if(power_control) powel_ctl(&last_charge_stop, &charge_is_stop);
                        sleep(read_one_option("CYCLE_TIME"));
                    }
                }
            }
            //向电流文件中写入最大充电电流
            if(current_change) set_array_value(current_max_file, current_max_file_num, current_max_char);
        }
        //如果手机未在充电，则只会刷新显示温度，以及若执行了获取前台应用包名的子线程，则取消此子线程
        else
        {
            //此处通过两个变量来确定上次循环时的充电状态以及是否是程序的第一次循环
            if(is_first_time)
            {
                printf_with_time("充电器未连接");
                is_first_time=0;
                last_charge_status=0;
            }
            else if(last_charge_status)
            {
                printf_with_time("充电器断开连接");
                last_charge_status=0;
            }
            //通过判断全局变量ForegroundAppName是否为空来判断获取前台应用包名的子线程是否在执行
            //若在执行，则取消此子线程，并清空ForegroundAppName
            pthread_mutex_lock((pthread_mutex_t *)&mutex_foreground_app);
            if(strlen((char *)ForegroundAppName))
            {
                pthread_mutex_unlock((pthread_mutex_t *)&mutex_foreground_app);
                printf_with_time("手机未在充电状态，“伪”旁路供电功能暂时停用");
                pthread_cancel(thread1);
                /*
                在子线程中，使用sleep(5)来实现5秒循环获取前台应用
                且在sleep(5)后紧跟pthread_testcancel()函数来检查子线程是否收到了取消信号
                此处等待1秒以确保子线程能够处于等待5秒的过程中或使子线程完成等待5秒
                这样在完成等待后就能直接在取消点取消子线程，从而使ForegroundAppName不会被子线程重新赋值
                */
                sleep(1);
                pthread_mutex_lock((pthread_mutex_t *)&mutex_foreground_app);
                memset((void *)ForegroundAppName, 0, sizeof(ForegroundAppName));
            }
            pthread_mutex_unlock((pthread_mutex_t *)&mutex_foreground_app);
            //此标识符代表手机是否处于“伪”旁路供电模式
            //如果在停止充电前手机处于“伪”旁路供电模式，则停止充电后将此标识符置0
            if(is_bypass) is_bypass=0;
            //因为断电也有可能是由电量控制引起的，所以此处执行电量控制函数
            if(power_control) powel_ctl(&last_charge_stop, &charge_is_stop);
            //此标识符代表手机是否通过此程序修改了电池温度
            //如果修改了，则电池温度无法再自动刷新，需要通过此程序进行刷新
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
