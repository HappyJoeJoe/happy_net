#ifndef __CO_THREAD_H__
#define __CO_THREAD_H__

//linux header
#include <ucontext.h>
#include <sys/types.h>
// #define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

//libc header
#include <stdio.h>
#include <string.h>

//c++ header
#include <vector>
#include <memory>

//user defined header
#include "../base/singleton.h"

using namespace std;

#define DEFAULT_SCHEDULE_SIZE 	128

#define DEFAULT_THREAD_SIZE 	1024

#define DEFAULT_STACK_SIZE 		(1024 * 128)

#define gettid() 				syscall(__NR_gettid)  

#define co_create(func, arg) 	co_schedule::create(func, arg)

#define co_resume(id) 			co_schedule::resume(id)

#define co_yield() 				co_schedule::yield()

#define co_release() 			co_schedule::release()

#define co_cur()				co_schedule::cur()


class co_thread;
class co_schedule;
typedef class co_thread 			co_thread_t;
typedef class co_schedule 			co_schedule_t;
typedef vector<co_thread_t*> 		co_sche_stack_t;
typedef vector<co_thread_t*> 		co_threads_t;
typedef co_sche_stack_t::iterator 	sche_stack_ite;
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

	inline void set_func(co_func_t f) { func = f; }

	inline co_func_t get_func() { return func; }

	inline void set_arg(void* a) { arg = a; }

	inline void* get_arg() { return arg; }

	inline void set_stat(enum co_thread_stat s) { stat = s; }

	inline enum co_thread_stat get_stat() { return stat; }

	inline ucontext_t& get_ctx() { return ctx; }

	inline char* get_stack() { return stack; }

	inline size_t get_stack_size() { return sizeof(stack); }

	inline int get_id() { return id; }

	inline void set_id(int co_id) { id = co_id; }

private:
	int 					id;
	ucontext_t 				ctx;
	co_func_t 				func;
	void* 					arg;
	char 					stack[DEFAULT_STACK_SIZE];
	enum co_thread_stat		stat;
};


class co_schedule
{
public:
	explicit co_schedule():running(-1), max_index(DEFAULT_THREAD_SIZE), tid(gettid()) {}
	~co_schedule() {}

	static int create(co_func_t func, void* arg);

	static int resume(int id);

	static int yield();

	static int release();

	static void thread_body();

	static int cur();

public:

	int init_env();

	inline co_sche_stack_t& get_sche_stack() { return sche_stack; }

	inline co_threads_t& get_threads() { return threads; }

	inline int get_running() { return running; }

	inline void set_running(int r) { running = r; }

	inline void set_max_index(int m) { max_index = m; }

	inline int get_max_index() { return max_index; }

	inline co_thread_t& get_thread_main() { return thread_main; }

private:
	static co_schedule_t* g_co_thread_arr_per_thread[102400];

	co_thread_t 		thread_main;//main 执行流协程
	co_sche_stack_t		sche_stack;	//调度协程栈
	co_threads_t		threads;	//协程管理队列
	int 				running;	//sche_stack 专门使用，top 表示当前运行的协程
	int 				max_index;
	pid_t 				tid;
};

#endif