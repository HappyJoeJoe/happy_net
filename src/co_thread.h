#ifndef __CO_THREAD_H__
#define __CO_THREAD_H__

#include <ucontext.h>

#define DEFAULT_STACK_SIZE (1024 * 128)

#define co_create(func, arg) co_thread::create(func, arg)

#define co_resume(id) co_thread::resume(id)

#define co_yield() co_thread::yield()

#define co_release() co_thread::release()

class co_thread;
typedef class co_thread co_thread_t;

typedef void (*co_func_t)(void*);

enum co_thread_stat
{
	FREE,
	RUNNABLE,
	RUNNING,
	SUSPEND
};


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

	static int co_size();

private:
	ucontext_t 				ctx;
	co_func_t 				func;
	void* 					arg;
	char 					stack[DEFAULT_STACK_SIZE];
	enum co_thread_stat 	stat;
friend:
	co_schedule_t;
};

#endif