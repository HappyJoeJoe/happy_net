#include "epoll.h"

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	static p_epoll_wait_t my_epoll_wait = 0;
	if(my_epoll_wait == 0)
	{
		
	}
}