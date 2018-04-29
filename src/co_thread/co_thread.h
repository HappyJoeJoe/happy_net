#ifndef __CO_THREAD_H__
#define __CO_THREAD_H__

//linux header
#include <ucontext.h>

//libc header
#include <stdio.h>

//c++ header
#include <vector>
#include <memory>

//user defined header
#include "singleton.h"

using namespace std;

#define DEFAULT_THREAD_SIZE 	1024

#define DEFAULT_STACK_SIZE 		(1024 * 128)

#define co_create(func, arg) co_schedule::create(func, arg)

#define co_resume(id) co_schedule::resume(id)

#define co_yield() co_schedule::yield()

#define co_release() co_schedule::release()

#define co_cur() co_schedule::cur()

#define co_size() co_schedule::size()

class co_thread;
class co_schedule;
typedef class co_thread 		co_thread_t;
typedef vector<co_thread_t*> 	sche_stack_t;
typedef vector<co_thread_t*> 	co_id_t;
typedef sche_stack_t::iterator 	sche_stack_ite;
typedef class co_schedule 		co_schedule_t;
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

private:
	ucontext_t 				ctx;
	co_func_t 				func;
	void* 					arg;
	char 					stack[DEFAULT_STACK_SIZE];
	enum co_thread_stat		stat;
	friend co_schedule_t;
};


class co_schedule
{
public:
	explicit co_schedule():running(-1), max_index(DEFAULT_THREAD_SIZE) {}
	~co_schedule() {}

	static int create(co_func_t func, void* arg);

	static int resume(int id);

	static int yield();

	static int release();

	static int cur();

	static int size();

public:
	int running_co() { return running; }

	int init_env();

	void push();

	int pop();

// private:
	co_thread_t 		thread_main;
	sche_stack_t		sche_stack;
	co_id_t 			threads;
	int 				running;
	int 				max_index;
};

#endif