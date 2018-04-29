#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include "lock.h"

template<T>
class singleton
{
private:
	explicit singleton() {}
	~singleton() {}

public:
	static * get_instance()
	{
		if(NULL == t)
		{
			t = new T();
		}
		return t;
	}

private:
	static T* t;
};

#endif