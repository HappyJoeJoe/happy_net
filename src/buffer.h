#ifndef __VEC_BUFFER_H__
#define __VEC_BUFFER_H__

#include <vector>
#include <assert.h>
#include <algorithm>
#include <string.h>
#include <iostream>

using namespace std;

class buffer
{
public:
	typedef vector<char>::iterator buffer_it;

	explicit buffer() : 
		read_idx_(reserve_size), 
		write_idx_(reserve_size), 
		buf_(reserve_size + init_size) 
	{

	}

	int readable_size()
	{
		assert(write_idx_ - read_idx_ >= 0);
		return write_idx_ - read_idx_;
	}

	int writable_size()
	{
		return buf_.size() - write_idx_;
	}

	template<typename T>
	int peek(T& t) 
	{
		size_t len = sizeof(T);
		if(len <= readable_size())
		{
			copy(read_begin(),
			     read_begin() + len,
			     (char *)&t);

			return len;
		}
		
		return -1;
	}

	/* 
	 * params:
	 * len: C式字符串，表示去掉截止符'\0'
	 */
	int peek_string(size_t len, char* ptr)
	{
		if(len + 1 <= readable_size())
		{
			copy(read_begin(),
			     read_begin() + len,
			     ptr);

			return len;
		}

		return -1;
	}

	template<typename T>
	int get(T& t)
	{
		size_t len = sizeof(T);
		if(len <= readable_size())
		{
			copy(read_begin(),
			     read_begin() + len,
			     (char *)&t);

			read_idx_ += len;

			return len;
		}

		return -1;
	}

	/* 
	 * params:
	 * len: C式字符串，表示去掉截止符'\0'
	 */
	int get_string(size_t len, char* ptr)
	{
		if(len + 1 <= readable_size())
		{
			copy(read_begin(),
			     read_begin() + len + 1,
			     ptr);

			read_idx_ += (len + 1);
			return len;
		}

		return -1;
	}

	template<typename T>
	int append(const T& t)
	{
		size_t len = sizeof(T);
		if(len > writable_size())
		{
			make_space(len);
		}

		copy((char *)&t,
			 (char *)&t + len,
			 write_begin());

		write_idx_ += len;

		return len;
	}

	int append_string(const char* ptr)
	{
		size_t len = strlen(ptr);
		if(len + 1 > writable_size())
		{
			make_space(len + 1);
		}

		copy(ptr,
			 ptr + len + 1,
			 write_begin());

		write_idx_ += (len + 1);

		return len;
	}

	char* read_begin()
	{
		return &*buf_.begin() + read_idx_;
	}

	char* write_begin()
	{
		return &*buf_.begin() + write_idx_;
	}

	int read_fd(int fd);

	int write_fd(int fd);

	void debug_info()
	{
		cout << "reserve_size:[" << reserve_size << "] "
		     << "read_idx_:[" << read_idx_ << "] "
		     << "write_idx_:[" << write_idx_ << "] "
		     << "buf_.size:[" << buf_.size() << "] "
		     << endl;
	}

private:

	int has_read()
	{
		return read_idx_ - reserve_size;
	}

	int make_space(size_t len)
	{
		assert(read_idx_ >= reserve_size);

		/* 已读区域 + 待写区域 >= 待写入数据长度 */
		if(has_read() + writable_size() >= len)
		{
			int origin_readable_size = readable_size();
			copy(read_begin(), 
				 read_begin() + origin_readable_size,
				 &*buf_.begin() + reserve_size);

			read_idx_ = reserve_size;
			write_idx_ = read_idx_ + origin_readable_size;
		}
		else
		{
			buf_.resize(write_idx_ + len + init_size);
		}

		return len;
	}

private:
	vector<char> buf_;
	int read_idx_;
	int write_idx_;

	const static int reserve_size = 8;   //预留字段
	const static int init_size    = 512; //初始化buf大小

	
};


#endif