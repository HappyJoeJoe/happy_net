#include "co_thread.h"

int co_schedule::create(co_func_t func, void* arg)
{
	int id = 0;
	co_thread_t* t = NULL;
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();
	if(-1 == s->running_co())
	{
		s->init_env();
	}

	for (id = 0; id < s->max_index; ++id)
	{
		if(s->threads[id]->stat == FREE)
		{
			break;
		}
	}

	if(id == s->max_index)
		return -1;

	t = s->threads[id];
	t->stat = RUNNABLE;
	t->func = func;
	t->arg 	= arg;

	return id;
}

void co_schedule::thread_body()
{
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();
	// printf("thread_body id:%d\n", id);
	co_thread_t* t = s->threads[s->running_co()];
	t->func(t->arg);
	t->stat = FREE;
	s->sche_stack.pop_back();
	s->running--;
}

int co_schedule::resume(int id)
{
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();
	if(id < 1 || id > s->max_index) return -1;

	co_thread_t* pre = s->threads[s->running_co()];
	co_thread_t* cur = s->threads[id];

	// printf("stat:%d\n", cur->stat);
	if(cur->stat == RUNNING || cur->stat == FREE) return -2;

	// cur->stat should be either 'RUNNABLE' or 'SUSPEND'
	if(cur->stat == RUNNABLE)
	{
		getcontext(&cur->ctx);
		cur->ctx.uc_stack.ss_sp		= cur->stack;
		cur->ctx.uc_stack.ss_size 	= sizeof(cur->stack);
		cur->ctx.uc_stack.ss_flags 	= 0;
		cur->ctx.uc_link			= &(pre->ctx);

		makecontext(&cur->ctx, co_schedule::thread_body, 0);
	}

	pre->stat = SUSPEND;
	cur->stat = RUNNING;

	s->sche_stack.push_back(cur);
	s->running++;

	swapcontext(&pre->ctx, &cur->ctx);

	return 0;
}

int co_schedule::yield()
{
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();

	co_thread_t* cur = s->threads[s->running_co()];

	if(cur == &s->thread_main) 
		return -1;

	cur->stat = SUSPEND;
	s->sche_stack.pop_back();
	s->running--;
	co_thread_t* pre = s->threads[s->running_co()];
	pre->stat = RUNNING;
	swapcontext(&cur->ctx, &pre->ctx);

	return 0;
}

int co_schedule::release()
{
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();
	sche_stack_ite it = s->threads.begin();
	for(; it != s->threads.end(); it++)
	{
		if((*it)->stat != FREE)
		{
			return -1;
		}
	}

	it = s->threads.begin();
	for(; it != s->threads.end(); it++)
	{
		if(*it != &s->thread_main)
		{
			delete *it;
		}
	}

	s->running = -1;

	return 0;
}

int co_schedule::init_env()
{
	threads.reserve(DEFAULT_THREAD_SIZE);
	sche_stack.reserve(DEFAULT_THREAD_SIZE);
	max_index = DEFAULT_THREAD_SIZE;
	int size = threads.capacity();
	for (int i = 0; i < size; i++)
	{
		co_thread_t* t = new co_thread_t();
		threads[i] = t;
		t->stat = FREE;
	}

	running = 0;
	thread_main.stat = RUNNING;
	sche_stack[running]	= &thread_main;
	threads[running]	= &thread_main;

	return 0;
}