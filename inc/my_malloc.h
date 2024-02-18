#ifndef _MY_MALLOC_H
#define _MY_MALLOC_H

extern void *(*my_calloc)(size_t, size_t);
extern void *(*my_realloc)(void *, size_t);
extern void (*my_free)(void *);

void free_malloc_memory(char ***addr, int num);

#endif
