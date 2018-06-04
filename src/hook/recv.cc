#include "recv.h"

ssize_t recv(int fd, void *buf, size_t count);
{
	if(sys_recv == 0)
	{
		sys_recv = (sys_read_t)dlsym(RTLD_NEXT, "recv");
	}

	int cur_co_id = co_cur();

	struct epoll_event ee = 
	{
		EPOLLIN | EPOLLRDHUP,
		cur_co_id
	};

	int ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ee);
	if(0 != ret)
	{
		ret = errno;
		printf("epoll_ctl add event false, efd:%d, op:%d, fd:%d, errno:%d",
			efd, op, fd, errno);
		return -1;
	}

	co_yield();

	ret = sys_recv(fd, buf, count);
	if(0 != ret)
	{
		ret = errno;
		printf("recv err, errno:%d\n", ret);
	}

	return ret;
}