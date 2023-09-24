#include "pthread.h"

#include "my_thread.h"

//创建两个互斥锁，供多线程使用
pthread_mutex_t mutex_foreground_app,mutex_options;
