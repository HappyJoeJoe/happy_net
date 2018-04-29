#include "co_thread.h"

int co_schedule::create(co_func_t func, void* arg)
{
	int id = 0;
	co_thread_t* t = NULL;
	unique_ptr<co_schedule_t> cs_ptr(singleton<co_schedule_t>::get_instance());
	if(-1 == cs_ptr->running_co())
	{
		cs_ptr->init_env();
	}

	for (id = 0; id < max_index; ++id)
	{
		if(threads[id].stat != FREE)
		{
			break;
		}
	}

	t = threads[id];
	t->stat = RUNNABLE;
	t->func = func;
	t->arg 	= arg;
	t->ctx.uc_stack.ss_sp 	= t->stack;
	t->ctx.uc_stack.ss_size = sizeof(t->stack);
	t->ctx.uc_link	 		= &sche_stack.at(running);

	return id;
}

int co_schedule::resume(int id)
{
	unique_ptr<co_schedule_t> cs_ptr(singleton<co_schedule_t>::get_instance());
	
	int running = cs_ptr.running_co();
	if(id < 1 || id > running) return -1;

	co_thread_t* t = threads[id];
	if(t->stat == RUNNING) return -2;
	co_thread_t* pre = sche_stack[running - 1];
	co_thread_t* cur = sche_stack[id];
	swapcontext(pre, curr);

	return id;
}

int co_schedule::yield()
{
	return 0;
}

int co_schedule::release()
{
	return 0;
}

int co_schedule::cur()
{
	return 0;
}

int co_schedule::size()
{
	return 0;
}

int co_schedule::init_env()
{
	threads.reserve(DEFAULT_THREAD_SIZE);
	sche_stack_ite it = threads.begin();
	for (; it != threads.end(); ++it)
	{
		co_thread_t* t = new co_thread_t();
		it->stat = FREE;
	}

	running = 0;
	sche_stack[running] = &thread_main;
	threads[running] = &thread_main;

	return 0;
}

void co_schedule::push(co_thread_t* t)
{
	sche_stack.push_back(t);
}

int co_schedule::pop()
{
	sche_stack.pop_back();
}