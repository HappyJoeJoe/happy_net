#ifndef __VEC_BUFFER_H__
#define __VEC_BUFFER_H__

/* unix header */
#include <unistd.h>

/* c header */
#include <vector>
#include <assert.h>
#include <algorithm>
#include <string.h>
#include <iostream>

/* user defined header */
#include "error.h"

using namespace std;

class buffer
{
public:
	explicit buffer() : 
		read_idx_(reserve_size), 
		write_idx_(reserve_size), 
		buf_(reserve_size + init_size) 
	{
		uint64_t* pointer = (uint64_t *)&*buf_.begin();
		*pointer = 0;
	}

	void reset()
	{
		read_idx_ = reserve_size;
		write_idx_ = reserve_size;

		uint64_t* pointer = (uint64_t *)&*buf_.begin();
		*pointer = 0;
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
		if(len <= readable_size())
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

			add_read_idx(len);

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
		if(len <= readable_size())
		{
			copy(read_begin(),
			     read_begin() + len,
			     ptr);

			add_read_idx(len);

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

		add_write_idx(len);

		return len;
	}

	/* 把去掉'\0'截止符的字符串放进去 */
	int append_string(const string& str)
	{
		return append_string(str.length(), str.c_str());
	}

	/* 把去掉'\0'截止符的字符串放进去 */
	int append_string(const char* ptr)
	{
		return append_string(strlen(ptr), ptr);
	}

	char* read_begin() { return &*buf_.begin() + read_idx_; }

	const char* read_begin() const { return &*buf_.begin() + read_idx_; }

	char* write_begin() { return &*buf_.begin() + write_idx_; }

	const char* write_begin() const { return &*buf_.begin() + write_idx_; }

	int readable_size()
	{
		assert(write_idx_ - read_idx_ >= 0);
		return write_idx_ - read_idx_;
	}

	int writable_size()
	{
		return buf_.size() - write_idx_;
	}
	
	int read_buf(int fd, int& err);

	int write_buf(int fd, int& err);

	int read_len(size_t len)
	{
		add_read_idx(len);
	}

	int write_len(size_t len)
	{
		add_write_idx(len);
	}

	void debug_info()
	{
		cout << "reserve_size:[" << reserve_size << "] "
		     << "read_idx_:[" << read_idx_ << "] "
		     << "write_idx_:[" << write_idx_ << "] "
		     << "buf_.size:[" << buf_.size() << "] "
		     << endl;
	}

	char* find_eof(const char* eof)
	{
		return strstr(read_begin(), eof);
	}

	uint64_t peek_reserve()
	{
		uint64_t* pointer = (uint64_t *)&*buf_.begin();
		return *pointer;
	}

private:
	void write_reserve(size_t len)
	{
		uint64_t* pointer = (uint64_t *)&*buf_.begin();
		*pointer += len;
	}

	void read_reserve(size_t len)
	{
		uint64_t* pointer = (uint64_t *)&*buf_.begin();
		*pointer -= len;
	}

	int append_string(size_t len, const char* ptr)
	{
		if(len > writable_size())
		{
			make_space(len);
		}

		copy(ptr,
			 ptr + len,
			 write_begin());

		add_write_idx(len);

		return len;
	}

	int has_read()
	{
		return read_idx_ - reserve_size;
	}

	int read_once(int fd, char* buf, int size);

	int write_once(int fd, char* buf, int size);

	void add_write_idx(size_t len)
	{
		write_idx_ += len;

		write_reserve(len);
	}

	void add_read_idx(size_t len)
	{
		read_idx_ += len;

		read_reserve(len);
	}

	int make_space(size_t len)
	{
		assert(read_idx_ >= reserve_size);

		/* 已读区域 + 待写区域 >= 待写入数据长度
		 * 则将可读区域重新移到 begin + reserve_size 处 */
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

	const static int reserve_size = 8;       //预留字段
	const static int init_size    = 125;    //初始化buf大小
	const static int max_buffer_read = 16 * 1024;
};


#endif