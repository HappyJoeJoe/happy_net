#ifndef __CO_THREAD_H__
#define __CO_THREAD_H__

#include <ucontext.h>

//栈大小 128 KB
#define MAX_STACK_SIZE (1024 * 128)

typedef co_thread_s co_thread_t;

typedef (void*)(*co_func_t)(void*);

typedef struct co_thread_s
{
	ucontext 		ctx
	co_func_t 		func;
	void* 			arg;
	char 			stack[MAX_STACK_SIZE];
	co_thread_stat 	stat;
};

enum co_thread_stat
{
	int RUNNABLE;
	int RUNNING;
	int YIELD;
};

int co_thread_create();

int co_thread_resume();

int co_thread_yield();

int co_thread_finish();

#endif