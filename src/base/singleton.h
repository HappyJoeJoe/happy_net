#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include "lock.h"

template<typename T>
class singleton
{
private:
	explicit singleton() {}
	~singleton() {}

public:
	//thread safe
	static T* get_instance()
	{
		static T* t = NULL;
		if(NULL == t)
		{
			// lock_t lock;
			// {
				if(NULL == t)
				{
					t = new T();
				}
			// }
		}
		return t;
	}
};

#endif