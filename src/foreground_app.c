#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

#include "printf_with_time.h"
#include "options_linkedlist.h"
#include "foreground_app.h"

//全局变量，用于存储当前前台应用的包名
volatile char ForegroundAppName[100]={0};

//读取小型文件内容到缓冲区，返回读取的字节数，失败返回-1
static int read_small_file(const char *path, char *buf, int bufsize)
{
    int fd=open(path, O_RDONLY);
    if(fd < 0) return -1;
    int n=read(fd, buf, bufsize-1);
    close(fd);
    if(n < 0) return -1;
    buf[n]='\0';
    return n;
}

//通过sysfs检测屏幕是否开启，返回1为开启，0为关闭
static int is_screen_on()
{
    char buf[32]={0};
    //尝试常见的LCD背光路径
    if(read_small_file("/sys/class/leds/lcd-backlight/brightness", buf, sizeof(buf)) > 0)
        return atoi(buf) > 0;
    //扫描/sys/class/backlight/目录下的所有背光设备
    DIR *dir=opendir("/sys/class/backlight");
    if(dir)
    {
        struct dirent *entry;
        char path[256];
        while((entry=readdir(dir)) != NULL)
        {
            if(entry->d_name[0] == '.') continue;
            snprintf(path, sizeof(path), "/sys/class/backlight/%s/brightness", entry->d_name);
            if(read_small_file(path, buf, sizeof(buf)) > 0)
            {
                int brightness=atoi(buf);
                closedir(dir);
                return brightness > 0;
            }
        }
        closedir(dir);
    }
    //无法检测屏幕状态，默认为开启
    return 1;
}

/*
通过扫描/proc目录获取前台应用包名
优先通过cgroup中的top-app标识判断（Android 7+的前台应用会被置于top-app cgroup）
后备方案为查找oom_score_adj值为0且UID>=10000的应用进程
*/
static int get_foreground_app(char *result, int result_size)
{
    DIR *proc_dir=opendir("/proc");
    if(!proc_dir) return -1;
    struct dirent *entry;
    char path[256], buf[1024], cmdline[APP_PACKAGE_NAME_MAX_SIZE];
    char fallback[APP_PACKAGE_NAME_MAX_SIZE]={0};
    struct stat st;
    while((entry=readdir(proc_dir)) != NULL)
    {
        //只处理数字目录（即PID）
        if(!isdigit((unsigned char)entry->d_name[0])) continue;
        //通过stat检查UID，UID>=10000为应用进程
        snprintf(path, sizeof(path), "/proc/%s", entry->d_name);
        if(stat(path, &st) != 0 || st.st_uid < 10000) continue;
        //读取cmdline获取进程名
        snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);
        if(read_small_file(path, cmdline, sizeof(cmdline)) <= 0) continue;
        //过滤非应用进程
        if(cmdline[0] == '/' || cmdline[0] == '[') continue;
        //去除多进程后缀（如com.example.app:service -> com.example.app）
        char *colon=strchr(cmdline, ':');
        if(colon) *colon='\0';
        //应用包名必须包含点号
        if(strchr(cmdline, '.') == NULL) continue;
        //检查cgroup中是否包含top-app标识（前台应用的特征）
        snprintf(path, sizeof(path), "/proc/%s/cgroup", entry->d_name);
        if(read_small_file(path, buf, sizeof(buf)) > 0 && strstr(buf, "/top-app"))
        {
            strncpy(result, cmdline, result_size-1);
            result[result_size-1]='\0';
            closedir(proc_dir);
            return 0;
        }
        //后备方案：记录oom_score_adj为0的应用进程
        if(!fallback[0])
        {
            snprintf(path, sizeof(path), "/proc/%s/oom_score_adj", entry->d_name);
            if(read_small_file(path, buf, sizeof(buf)) > 0 && atoi(buf) == 0)
                strncpy(fallback, cmdline, sizeof(fallback)-1);
        }
    }
    closedir(proc_dir);
    //cgroup方案未找到，使用oom_score_adj后备方案
    if(fallback[0])
    {
        strncpy(result, fallback, result_size-1);
        result[result_size-1]='\0';
        return 0;
    }
    return -1;
}

/*
获取前台应用的包名以配合“伪”旁路供电功能使用
通过扫描/proc目录与sysfs实现，纯C语言，不依赖popen或Shell命令
*/
void *get_foreground_appname(void *android_version)
{
    (void)android_version;
    while(read_one_option("BYPASS_CHARGE") == 1)
    {
        char result[APP_PACKAGE_NAME_MAX_SIZE]={0};
        //通过sysfs检测屏幕是否开启
        if(!is_screen_on())
        {
            pthread_mutex_lock((pthread_mutex_t *)&mutex_foreground_app);
            strncpy((char *)ForegroundAppName, "screen_is_off", APP_PACKAGE_NAME_MAX_SIZE-1);
            pthread_mutex_unlock((pthread_mutex_t *)&mutex_foreground_app);
            sleep(5);
            pthread_testcancel();
            continue;
        }
        //通过/proc获取前台应用包名
        if(get_foreground_app(result, sizeof(result)) != 0)
        {
            printf_with_time("无法获取前台应用包名，跳过本次循环！");
            sleep(5);
            pthread_testcancel();
            continue;
        }
        pthread_testcancel();
        //将前台应用包名赋值给ForegroundAppName
        pthread_mutex_lock((pthread_mutex_t *)&mutex_foreground_app);
        strncpy((char *)ForegroundAppName, result, APP_PACKAGE_NAME_MAX_SIZE-1);
        pthread_mutex_unlock((pthread_mutex_t *)&mutex_foreground_app);
        sleep(5);
        pthread_testcancel();
    }
    //如果配置文件的BYPASS_CHARGE值不为1，则退出循环并将ForegroundAppName清空
    pthread_mutex_lock((pthread_mutex_t *)&mutex_foreground_app);
    memset((void *)ForegroundAppName, 0, sizeof(ForegroundAppName));
    pthread_mutex_unlock((pthread_mutex_t *)&mutex_foreground_app);
    return NULL;
}

//通过读取/system/build.prop获取安卓版本
int check_android_version()
{
    int android_version=0;
    const char *prop_files[]={"/system/build.prop", "/vendor/build.prop", "/system/vendor/build.prop", NULL};
    const char key[]="ro.build.version.release=";
    for(int i=0;prop_files[i];i++)
    {
        FILE *fp=fopen(prop_files[i], "r");
        if(!fp) continue;
        char line[256];
        while(fgets(line, sizeof(line), fp))
        {
            line_feed(line);
            if(strstr(line, key) == line)
            {
                android_version=atoi(line+strlen(key));
                fclose(fp);
                goto version_found;
            }
        }
        fclose(fp);
    }
    printf_with_time("无法从属性文件中获取安卓版本，“伪”旁路供电功能失效！");
    return 0;
version_found:
    if(android_version < 7)
    {
        printf_with_time("安卓7及以下无法获取前台应用，当前版本为安卓%d，“伪”旁路供电功能失效！", android_version);
        return 0;
    }
    return android_version;
}
