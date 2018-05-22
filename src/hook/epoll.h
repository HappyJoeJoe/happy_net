#ifndef __EPOLL_H__
#define __EPOLL_H__

//用来 hook 系统调用
#include <sys/epoll.h>


int efd = epoll_create1(EPOLL_CLOEXEC);

typedef int (*p_epoll_wait_t)(int epfd, int op, int fd, struct epoll_event *event);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);



#endif