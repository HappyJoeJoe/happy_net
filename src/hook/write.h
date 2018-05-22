#ifndef __WRITE_H__
#define __WRITE_H__

//用来 hook 系统调用
#include <unistd.h>
#include "epoll.h"
#include "../co_thread/co_thread.h"

extern int efd;

typedef int (*sys_write_t)(int fd, void *buf, size_t count);
ssize_t write(int fd, void *buf, size_t count);

#endif