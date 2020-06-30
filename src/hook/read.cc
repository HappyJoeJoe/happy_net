#include "read.h"

sys_read = (sys_read_t)dlsym(RTLD_NEXT, "read");

ssize_t read(int fd, void *buf, size_t count);
{
	if(sys_read == 0)
	{
		sys_read = (sys_read_t)dlsym(RTLD_NEXT, "read");
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

	ret = sys_read(fd, buf, count);
	if(0 != ret)
	{
		ret = errno;
		printf("read err, errno:%d\n", ret);
	}

	return ret;
}