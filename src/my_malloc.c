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
