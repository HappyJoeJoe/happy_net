#ifndef __HOOK_H__
#define __HOOK_H__

#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>


typedef void *(*p_malloc_t)(size_t);

//用来 hook 系统调用
void *malloc(size_t);

#endif