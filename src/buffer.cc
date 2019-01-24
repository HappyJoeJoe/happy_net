#include "buffer.h"

/*
 * 边沿触发模式下，读取buffer的写法
 * 能一次性读完就一次性读完
 */
int buffer::et_read_fd(int fd)
{
	int ret = 0;

	while(0 != ret)
	{
		ret = read(fd, write_begin(), writable_size());
		if(-1 == ret)
		{
			if(EAGAIN == errno)
			{
				
			}
		}
		else if(0 == ret) //client close connection
		{
			printf("err:[%s]\n", strerror(errno));
			return ;
		}
		else
		{
			
		}
	}

	return 0;
}

/*
 * 水平触发模式下，读取buffer的写法
 */
int buffer::lt_read_fd(int fd)
{
	int ret = 0;
	ret = read(fd, write_begin(), writable_size());
	if(-1 == ret)
	{
		if(EAGAIN == errno || EWOULDBLOCK == errno)
		{

		}
	}
	else if()
	{

	}

	return 0;
}