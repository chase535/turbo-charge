#ifdef MI_MALLOC
#include "mimalloc.h"
void *(*my_calloc)(size_t, size_t)=&mi_calloc;
void *(*my_realloc)(void *, size_t)=&mi_realloc;
void (*my_free)(void *)=&mi_free;
#else
#include "malloc.h"
void *(*my_calloc)(size_t, size_t)=&calloc;
void *(*my_realloc)(void *, size_t)=&realloc;
void (*my_free)(void *)=&free;
#endif

#include "stdlib.h"

//完全释放动态申请的二级指针的内存
void free_malloc_memory(char ***addr, int num)
{
    if(addr != NULL && *addr != NULL)
    {
        if(!num) num=1;
        for(int i=0;i < num;i++)
        {
            if((*addr)[i] != NULL)
            {
                my_free((*addr)[i]);
                (*addr)[i]=NULL;
            }
        }
        my_free(*addr);
        *addr=NULL;
    }
}
