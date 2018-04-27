#ifndef __CO_THREAD_H__
#define __CO_THREAD_H__

#include <ucontext.h>
// #include "co_schedule.h"

#define DEFAULT_STACK_SIZE (1024 * 128)

#define co_create(func, arg) co_thread::create(func, arg)

#define co_resume(id) co_thread::resume(id)

#define co_yield() co_thread::yield()

#define co_release() co_thread::release()

#define co_curr() co_thread::curr()

#define co_size() co_thread::size()

class co_thread;
typedef class co_schedule co_schedule_t;
typedef class co_thread co_thread_t;
typedef void (*co_func_t)(void*);


class co_thread
{
public:
	explicit co_thread() {}
	~co_thread() {}

	static int create();

	static int resume();

	static int yield();

	static int release();

	static int curr();

	static int size();

private:
	enum co_thread_stat
	{
		FREE,
		RUNNABLE,
		RUNNING,
		SUSPEND
	};

	ucontext_t 				ctx;
	co_func_t 				func;
	void* 					arg;
	char 					stack[DEFAULT_STACK_SIZE];
	enum co_thread_stat 	stat;

private:
	static co_schedule_t	sche;
};

#endif