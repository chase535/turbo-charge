#include "my_thread.h"

//创建两个互斥锁，供多线程使用
volatile pthread_mutex_t mutex_foreground_app,mutex_options;
