#include "buffer.h"

int buffer::read_once(int fd, char* buf, int size)
{
	int err = 0;
	int n_read = 0;

	do
	{
		n_read = read(fd, buf, size);
		if(-1 == n_read)
		{
			err = errno;
			if(EAGAIN == err)
			{
				// printf("loc:[%s] line:[%d]  EAGAIN  buf:%s, n_read:%d\n", __func__, __LINE__, buf, n_read);
				return kBUFFER_EAGAIN;
			}
			else if(EINTR == err)
			{
			}
			else
			{
				return kBUFFER_ERROR;
			}
		}
		else if(n_read >= 0)
		{
			// printf("loc:[%s] line:[%d]  n_read>=0  buf:%s, n_read:%d\n", __func__, __LINE__, buf, n_read);
			break;
		}
		
	}while(err == EINTR); //未读取数据之前遇到信号中断，则重读

	return n_read;
}

int buffer::read_buf(int fd, int& err)
{
	int ret = 0;
	int count = 0;

	for (;;)
	{
		ret = read_once(fd, write_begin(), writable_size());
		if(kBUFFER_ERROR == ret)
		{
			printf("loc:[%s] line:[%d] fd:%d  read error \n", __func__, __LINE__, fd);
			err = kBUFFER_ERROR;
			return -1;
		}
		else if(kBUFFER_EAGAIN == ret)
		{
			err = kBUFFER_EAGAIN;

			if(count > 0)
			{
				break;
			}

			printf("loc:[%s] line:[%d] fd:%d  EAGAIN \n", __func__, __LINE__, fd);
			return -1;
		}
		else if(ret == 0)
		{
			err = kBUFFER_EOF;

			if(count > 0)
			{
				break;
			}

			printf("loc:[%s] line:[%d] fd:%d  eof \n", __func__, __LINE__, fd);
			return -1;
		}
		else
		{
			count += ret;
			set_write_idx(ret);

			if(writable_size() == 0)
			{
				printf("loc:[%s] line:[%d] fd:%d  make more space \n", __func__, __LINE__, fd);
				make_space(init_size);
			}

			// printf("方法:%s 行号:%d fd:%d *** has_read:%d ***\n", __func__, __LINE__, fd, ret);
		}
	}

	return count;
}
