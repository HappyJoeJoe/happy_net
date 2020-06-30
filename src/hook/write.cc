#include "write.h"

sys_write = (sys_read_t)dlsym(RTLD_NEXT, "write");

ssize_t write(int fd, void *buf, size_t count);
{
	if(sys_write == 0)
	{
		sys_write = (sys_write_t)dlsym(RTLD_NEXT, "write");
	}

	int cur_co_id = co_cur();

	struct epoll_event ee = 
	{
		EPOLLOUT,
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

	ret = sys_write(fd, buf, count);
	if(0 != ret)
	{
		ret = errno;
		printf("write err, errno:%d\n", ret);
	}

	return ret;
}