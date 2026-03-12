#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

#include "read_option_file.h"
#include "printf_with_time.h"

/*
读取配置文件并赋值给程序内部变量
*/
void *read_option_file(void *arg)
{
    int sleep_time=*(int *)arg, first_run=1;
    //监视配置文件所在目录而非文件本身，以正确处理编辑器的原子保存（先写临时文件再重命名）
    char opt_dir[PATH_MAX], opt_name[NAME_MAX+1];
    strncpy(opt_dir, option_file, sizeof(opt_dir)-1);
    opt_dir[sizeof(opt_dir)-1]='\0';
    char *slash=strrchr(opt_dir, '/');
    if(slash)
    {
        strncpy(opt_name, slash+1, sizeof(opt_name)-1);
        opt_name[sizeof(opt_name)-1]='\0';
        if(slash == opt_dir) { opt_dir[0]='/'; opt_dir[1]='\0'; }
        else *slash='\0';
    }
    else
    {
        strncpy(opt_name, option_file, sizeof(opt_name)-1);
        opt_name[sizeof(opt_name)-1]='\0';
        opt_dir[0]='.'; opt_dir[1]='\0';
    }
    int ifd=inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    int wd=-1;
    if(ifd >= 0)
    {
        wd=inotify_add_watch(ifd, opt_dir, IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE);
        if(wd < 0)
        {
            printf_with_time("inotify_add_watch 失败，降级为周期性轮询");
            close(ifd);
            ifd=-1;
        }
    }
    while(1)
    {
        if(!first_run)
        {
            if(ifd < 0)
            {
                //inotify 初始化失败，降级为周期性轮询
                sleep(sleep_time);
            }
            else
            {
                //使用 select 阻塞等待 inotify 事件，超时时间为 sleep_time 秒
                //超时后触发周期性兜底重载，以防 inotify 漏报
                fd_set rfds;
                struct timeval tv={sleep_time, 0};
                FD_ZERO(&rfds);
                FD_SET(ifd, &rfds);
                int sel=select(ifd+1, &rfds, NULL, NULL, &tv);
                if(sel < 0) continue;
                if(sel > 0)
                {
                    //读取 inotify 事件队列，仅当有与配置文件相关的事件时才重载
                    int relevant=0;
                    char evbuf[4096];
                    ssize_t n;
                    while((n=read(ifd, evbuf, sizeof(evbuf))) > 0)
                    {
                        char *ptr=evbuf;
                        while(ptr < evbuf+n)
                        {
                            struct inotify_event *ev=(struct inotify_event *)ptr;
                            if(ev->len > 0 && ev->name[0] != '\0' && strcmp(ev->name, opt_name) == 0) relevant=1;
                            ptr+=sizeof(struct inotify_event)+ev->len;
                        }
                    }
                    if(!relevant) continue;
                }
                //sel==0 为超时，直接落入下方进行周期性兜底重载
            }
        }
        //在循环体内定义变量，这样变量仅存在于单次循环，每次循环结束后变量自动释放，循环开始时变量重新定义
        char option_tmp[42]={0},option[100]={0},*value=NULL;
        uchar opt=0,i=0,option_quantity=1;
        int new_value=0;
        struct stat statbuf;
        ListNode *node=options_head.next,*tmp=options_head.next;
        if(node == NULL || tmp == NULL)
        {
            printf_with_time("存储配置信息的链表中没有节点，程序强制停止运行！");
            exit(9988);
        }
        while((tmp=tmp->next) != NULL) option_quantity++;
        //不定长数组无法在定义时初始化为全0，所以后续会使用memset进行清零
        uchar value_stat[option_quantity];
        check_read_file(option_file);
        memset(value_stat, 0, sizeof(value_stat));
        pthread_mutex_lock((pthread_mutex_t *)&mutex_options);
        int fd=open(option_file, O_RDONLY);
        if(fd == -1)
        {
            pthread_mutex_unlock((pthread_mutex_t *)&mutex_options);
            continue;
        }
        if(fstat(fd, &statbuf) == -1)
        {
            close(fd);
            pthread_mutex_unlock((pthread_mutex_t *)&mutex_options);
            continue;
        }
        if(statbuf.st_size > 0)
        {
            char *map=mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            close(fd);
            if(map == MAP_FAILED)
            {
                pthread_mutex_unlock((pthread_mutex_t *)&mutex_options);
                continue;
            }
            char *pos=map, *end=map+statbuf.st_size, *line_end;
            while(pos < end)
            {
                line_end=memchr(pos, '\n', end-pos);
                size_t line_len=line_end ? (size_t)(line_end-pos) : (size_t)(end-pos);
                if(line_len >= sizeof(option)) line_len=sizeof(option)-1;
                memcpy(option, pos, line_len);
                option[line_len]='\0';
                pos=line_end ? line_end+1 : end;
                line_feed(option);
                //跳过以英文井号开头的行及空行
                if((strlen(option) == 0) || (option[0] == '#')) continue;
                for(opt=0,node=options_head.next;node;opt++,node=node->next)
                {
                    //将配置名与等号进行拼接，用来进行匹配
                    snprintf(option_tmp, 42, "%s=", node->name);
                    if(strstr(option, option_tmp) == NULL) continue;
                    //value_stat数组存储配置文件中每个变量值的状态，详情直接看后面判断value_stat数组的值的相关代码
                    value_stat[opt]=10;
                    //判断变量值是否合法，合法则对程序内部变量进行赋值，否则将状态存入value_stat数组中
                    if(!strcmp(option, option_tmp)) value_stat[opt]=1;
                    else
                    {
                        value=option+strlen(option_tmp);
                        for(i=0;i < strlen(value);i++)
                        {
                            if((value[i] < '0' || value[i] > '9') && ((!i && (value[i] != '-' || strlen(value) == 1)) || i))
                            {
                                value_stat[opt]=2;
                                break;
                            }
                        }
                    }
                    if(value_stat[opt] != 10) continue;
                    new_value=atoi(value);
                    if(new_value < 0) value_stat[opt]=3;
                    else if(!strcmp((char *)(node->name), "CYCLE_TIME") && !new_value) value_stat[opt]=4;
                    if(value_stat[opt] == 10 && node->value != new_value)
                    {
                        node->value=new_value;
                        value_stat[opt]=100;
                    }
                }
            }
            munmap(map, statbuf.st_size);
        }
        else
        {
            close(fd);
        }
        for(opt=0,node=options_head.next;node;opt++,node=node->next)
        {
            //通过判断是否第一次运行来进行不同的字符串输出
            if(!first_run)
            {
                if(value_stat[opt] == 0) printf_with_time("配置文件中%s不存在，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 1) printf_with_time("新%s的值为空，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 2) printf_with_time("新%s的值不是由纯数字组成，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 3) printf_with_time("新%s的值小于0，这是不被允许的，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 4) printf_with_time("新%s的值为0，这是不被允许的，故程序沿用上一次的值%d", node->name, node->value);
                else if(value_stat[opt] == 100) printf_with_time("%s的值更改为%d", node->name, node->value);
            }
            else
            {
                if(value_stat[opt] == 0) printf_with_time("配置文件中%s不存在，故程序使用默认值%d", node->name, node->value);
                else if(value_stat[opt] == 1) printf_with_time("配置文件中%s的值为空，故程序使用默认值%d", node->name, node->value);
                else if(value_stat[opt] == 2) printf_with_time("配置文件中%s的值不是由纯数字组成，故程序使用默认值%d", node->name, node->value);
                else if(value_stat[opt] == 3) printf_with_time("配置文件中%s的值小于0，这是不被允许的，故程序使用默认值%d", node->name, node->value);
                else if(value_stat[opt] == 4) printf_with_time("配置文件中%s的值为0，这是不被允许的，故程序使用默认值%d", node->name, node->value);
            }
        }
        first_run=0;
        pthread_mutex_unlock((pthread_mutex_t *)&mutex_options);
    }
    return NULL;
}
