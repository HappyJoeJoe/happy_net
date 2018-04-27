#ifndef __CO_SCHEDULE_H__
#define __CO_SCHEDULE_H__

#include <ucontext.h>
#include <vector>
#include "co_thread.h"
using namespace std;

#define DEFAULT_THREAD_SIZE 1024
typedef std::vector<co_thread_t> co_stack_t;

class co_schedule;
typedef class co_schedule co_schedule_t;

class co_schedule
{
public:
	
private:
	co_thread_t thread_main;
	co_stack_t stack;
};

#endif