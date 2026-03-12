#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
    int ifd=inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    int iwd=-1;
    if(ifd == -1)
        printf_with_time("inotify_init1 失败 (%s)，回退到周期性轮询", strerror(errno));
    else
    {
        iwd=inotify_add_watch(ifd, option_file, IN_CLOSE_WRITE | IN_MODIFY);
        if(iwd == -1)
            printf_with_time("inotify_add_watch 失败 (%s)，回退到周期性轮询", strerror(errno));
    }
    while(1)
    {
        if(!first_run)
        {
            //ifd 有效时尝试使用 inotify 等待事件，否则回退到周期性轮询
            if(ifd != -1)
            {
                //监视描述符无效时尝试重新添加监视（文件被删除重建后 iwd 会置为 -1）
                if(iwd == -1)
                {
                    iwd=inotify_add_watch(ifd, option_file, IN_CLOSE_WRITE | IN_MODIFY);
                    if(iwd == -1)
                        printf_with_time("inotify_add_watch 失败 (%s)，回退到周期性轮询", strerror(errno));
                }
            }
            if(ifd != -1 && iwd != -1)
            {
                //使用 select 阻塞等待 inotify 事件，超时时间为 sleep_time 秒
                //文件未修改时线程持续休眠，不产生任何系统调用开销
                fd_set rfds;
                struct timeval tv={sleep_time, 0};
                FD_ZERO(&rfds);
                FD_SET(ifd, &rfds);
                if(select(ifd+1, &rfds, NULL, NULL, &tv) <= 0) continue;
                //排空 inotify 事件队列，防止积压，并检测监视是否被自动移除
                char evbuf[sizeof(struct inotify_event)+NAME_MAX+1];
                ssize_t nread;
                while((nread=read(ifd, evbuf, sizeof(evbuf))) > 0)
                {
                    char *p=evbuf, *pend=evbuf+nread;
                    while(p+(ssize_t)sizeof(struct inotify_event) <= pend)
                    {
                        struct inotify_event *ev=(struct inotify_event *)p;
                        //整个事件（含名称）超出缓冲区时停止解析
                        if(p+(ssize_t)(sizeof(struct inotify_event)+(size_t)ev->len) > pend) break;
                        //IN_IGNORED 表示监视已被内核自动移除（如文件被删除），标记 iwd 失效
                        if(ev->mask & IN_IGNORED) iwd=-1;
                        p+=sizeof(struct inotify_event)+ev->len;
                    }
                }
            }
            else
                sleep(sleep_time);
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
        fstat(fd, &statbuf);
        if(statbuf.st_size > 0)
        {
            char *map=mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            close(fd);
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
