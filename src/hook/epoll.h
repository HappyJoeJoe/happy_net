#ifndef __EPOLL_H__
#define __EPOLL_H__

//用来 hook 系统调用
#include <sys/epoll.h>

typedef int (*p_epoll_wait_t)(int epfd, struct epoll_event *events, int maxevents, int timeout);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#endif