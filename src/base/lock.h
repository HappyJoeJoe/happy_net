#ifndef __LOCK_H__
#define __LOCK_H__

//linux header
#include <pthread.h>

class lock;
class lock_api;
typedef class lock 		lock_t;
typedef class lock_api 	lock_api_t;

class lock_api
{
public:
	explicit lock_api(pthread_mutex_t& m) : mutex(m) 
	{
		pthread_mutex_init(&mutex, 0);
	}
	~lock_api() 
	{
		pthread_mutex_destroy(&mutex);
	}

	void lock()
	{
		pthread_mutex_lock(&mutex);
	}

	void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}

	void try_lock()
	{
		pthread_mutex_trylock(&mutex);
	}

private:
	pthread_mutex_t& mutex;

};

class lock
{
public:
	explicit lock(pthread_mutex_t& mutex) : api(mutex)
	{
		api.lock();
	}
	~lock()
	{
		api.unlock();
	}

private:
	lock_api api;
};

class try_lock
{
public:
	explicit try_lock(pthread_mutex_t& mutex) : api(mutex)
	{
		api.try_lock();
	}
	~try_lock()
	{
		api.unlock();		
	}

private:
	lock_api api;
};

#endif