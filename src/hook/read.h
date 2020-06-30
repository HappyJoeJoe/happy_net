#ifndef __READ_H__
#define __READ_H__

//用来 hook 系统调用
#include <unistd.h>
#include "epoll.h"
#include "../co_routinue.h"

extern int efd;

typedef int (*sys_read_t)(int fd, void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);

typedef struct sys_read_arg_t 
{
	int fd;
	void* buf;
	size_t count
} sys_read_arg_s;


#endif