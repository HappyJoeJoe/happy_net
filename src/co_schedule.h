#ifndef __CO_SCHEDULE_H__
#define __CO_SCHEDULE_H__

#include <ucontext.h>
#include <vector>
#include "co_thread.h"
using namespace std;

#define DEFAULT_THREAD_SIZE 1024
typedef vector<co_thread_t> 	co_stack_t;
typedef vector<co_thread_t*> 	co_id_t;

class co_schedule;
typedef class co_schedule co_schedule_t;

class co_schedule
{
public:
	explicit co_schedule() {}
	~co_schedule() {}

private:


private:
	co_thread_t 		thread_main;
	co_stack_t 			sche_stack;
	co_id_t 			sche_id;
	int 				running;
	int 				max_index;

};

#endif