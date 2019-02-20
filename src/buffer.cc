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
				// info_log("%s|%s|%d  EAGAIN  n_read:%d\n", __FILE__, __func__, __LINE__, n_read);
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
			// info_log("%s|%s|%d  n_read >= 0  n_read:%d\n", __FILE__, __func__, __LINE__, n_read);
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
			// info_log("%s->%s:%d  fd:%d  read error\n", __FILE__, __func__, __LINE__, fd);
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

			// info_log("%s->%s:%d  fd:%d  EAGAIN \n", __FILE__, __func__, __LINE__, fd);
			return -1;
		}
		else if(ret == 0)
		{
			err = kBUFFER_EOF;

			if(count > 0)
			{
				break;
			}

			// printf("%s->%s:%d  fd:%d  eof \n", __FILE__, __func__, __LINE__, fd);
			return -1;
		}
		else
		{
			count += ret;
			set_write_idx(ret);

			if(writable_size() == 0)
			{
				// printf("[%s->%s:%d]  fd:%d  make more space \n", __FILE__, __func__, __LINE__, fd);
				make_space(init_size);
			}

			// printf("方法:%s 行号:%d fd:%d *** has_read:%d ***\n", __func__, __LINE__, fd, ret);
		}
	}

	return count;
}

int buffer::write_once(int fd, char* buf, int size)
{
	int err = 0;
	int n_write = 0;

	do
	{
		n_write = write(fd, buf, size);
		if(-1 == n_write)
		{
			err = errno;
			if(EAGAIN == err)
			{
				// printf("[%s->%s:%d]  EAGAIN  n_write:%d\n", __FILE__, __func__, __LINE__, n_write);
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
		else if(n_write >= 0)
		{
			// printf("[%s->%s:%d]  ======>  n_write:%d\n", __FILE__, __func__, __LINE__, n_write);
			break;
		}
		
	}while(err == EINTR); //未读取数据之前遇到信号中断，则重读

	return n_write;
}

int buffer::write_buf(int fd, int& err)
{
	int ret = 0;
	int count = 0;

	for (;;)
	{
		ret = write_once(fd, read_begin(), readable_size());
		if(kBUFFER_ERROR == ret)
		{
			// printf("%s->%s:%d  fd:%d  read error\n", __FILE__, __func__, __LINE__, fd);
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

			// printf("%s->%s:%d  fd:%d  EAGAIN \n", __FILE__, __func__, __LINE__, fd);
			return -1;
		}
		else if(ret == 0)
		{
			err = 0;

			if(count > 0)
			{
				break;
			}

			// printf("%s->%s:%d  fd:%d  eof \n", __FILE__, __func__, __LINE__, fd);
			return 0;
		}
		else
		{
			count += ret;
			set_read_idx(ret);
			// printf("方法:%s 行号:%d fd:%d *** has_read:%d ***\n", __func__, __LINE__, fd, ret);
		}
	}

	return count;
}