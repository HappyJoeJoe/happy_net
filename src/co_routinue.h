/**********************************
 *@项目名称: 协程库
 *@文件名称: co_thread.h
 *@Date 2018-05-16 18:00:00
 *@Author jiabo.zhou
 *@Copyright（C）: 2014-2020 fancy Inc.   All rights reserved.
 *注意：
***************************************/

#ifndef __CO_ROUTINUE_H__
#define __CO_ROUTINUE_H__

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
#include <map>
#include <memory>

//user defined header
#include "singleton.h"

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


class co_routinue;
class co_schedule;
typedef class co_routinue 			co_routinue_t;
typedef class co_schedule 			co_schedule_t;
typedef vector<co_routinue_t*> 		co_sche_stack_t;
typedef vector<co_routinue_t*> 		co_threads_t;
typedef co_sche_stack_t::iterator 	sche_stack_iter;
typedef map<int, co_schedule_t*>::iterator	g_co_routinue_per_thread_iter;
typedef void (*co_func_t)(void*);

enum co_thread_stat
{
	FREE,
	RUNNABLE,
	RUNNING,
	SUSPEND
};

class co_routinue
{
public:
	explicit co_routinue() {}
	~co_routinue() {}

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
	explicit co_schedule():running(-1), tid(gettid()) {}
	~co_schedule() {}

	static int create(co_func_t func, void* arg);

	static int resume(int id);

	static int yield();

	static int release();

	static void thread_body();

	static int cur();

private:
	int init_env();

private:
	static map<int, co_schedule_t*> g_co_routinue_per_thread;

	co_routinue_t 		thread_main;//main 执行流协程
	co_sche_stack_t		sche_stack;	//调度协程栈
	co_threads_t		threads;	//协程管理队列
	int 				running;	//sche_stack 专门使用，top 表示当前运行的协程
	pid_t 				tid;
};

#endif