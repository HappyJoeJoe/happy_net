#include "epoll.h"

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
{
	static p_epoll_wait_t my_epoll_ctl = 0;
	if(my_epoll_ctl == 0)
	{
		my_epoll_ctl = (p_epoll_wait_t)dlsym("epoll_ctl");
	}
}