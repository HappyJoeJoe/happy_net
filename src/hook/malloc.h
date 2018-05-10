#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <stdlib.h>


typedef void *(*p_malloc_t)(size_t);

//用来 hook 系统调用
void *malloc(size_t);

#endif