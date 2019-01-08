#ifndef __BUFFER_H__
#define __BUFFER_H__

#include<vector>
#include<cassert>
#include<algtrithm>

using namespace std;

class buffer
{
public:
	explicit buffer() : 
		read_idx_(reserve_size), 
		write_idx_(reserve_size), 
		buf_(init_size) 
	{

	}

	int readable_size()
	{
		asset(write_idx_ - read_idx_ > 0);
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
		if(len < readable_size())
		{
			t = *static_cast<T*>(&*buf_.begin() + read_idx_);
			return len;
		}
		
		return -1;
	}

	int peek_string(size_t len, string& str)
	{
		if(len < readable_size())
		{
			str = buf_.begin() + read_idx_;
			return len;
		}

		return -1;
	}

	template<typename T>
	int get(T& t)
	{
		size_t len = sizeof(T);
		if(len < readable_size())
		{
			t = *static_cast<T*>(&*buf_.begin() + read_idx_);
			read_idx_ += len;
			return len;
		}

		return -1;
	}

	int get_string(size_t len, string& str)
	{
		if(len < readable_size())
		{
			str = buf_.begin() + read_idx_;
			read_idx_ += len;
			return len;
		}

		return -1;
	}

	template<typename T>
	int append(T& t)
	{
		size_t len = sizeof(T);
		if(len > writable_size())
		{
			make_space(len);
		}

		T* t_p = static_cast<T*>(&*buf_.begin() + write_idx_)
		*t_p = t;

		return len;
	}

	int append_string(const string& str)
	{
		size_t len = str.length();
		if(len > writable_size())
		{
			make_space(len);
		}

		copy(buf_.begin() + write_idx_,
			 buf_.begin() + write_idx_ + len,
			 str.begin());

		return len;
	}

	int read_fd(int fd);

private:
	int make_space(size_t len)
	{
		assert(read_idx_ >= reserve_size);

		if(read_idx_ - reserve_size /* 已读区域 */
			+ writable_size()       /* 可写其余 */
			>= len)
		{
			int old_readable_size = readable_size();
			copy(buf_.begin() + read_idx_, 
				 buf_.begin() + read_idx_ + old_readable_size,
				 buf_.begin() + reserve_size);

			read_idx_ = reserve_size;
			write_idx_ = read_idx_;

			return len;
		}
		else
		{
			int old_readable_size = readable_size();
			vector<char> b;
			b.resize(reserve_size + old_readable_size + len + init_size*2);
			buffer_it = copy(buf_.begin() + read_idx_,
				 buf_.begin() + read_idx_ + old_readable_size,
				 b.begin() + reserve_size);
			buf_.swap(b);
			read_idx_ = reserve_size;
			write_idx_ = read_idx_ + old_readable_size;
		}

		return len;
	}

private:
	vector<char> buf_;
	int read_idx_;
	int write_idx_;

	const static int reserve_size = 8; //预留字段
	const static int init_size = 1024; 

	typedef vector<char>::iteraotr buffer_it;
};


#endif