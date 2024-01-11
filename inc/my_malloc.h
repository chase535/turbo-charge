#ifndef _MY_MALLOC_H
#define _MY_MALLOC_H

void *(*my_calloc)(size_t, size_t);
void *(*my_realloc)(void *, size_t);
void (*my_free)(void *);

#endif
