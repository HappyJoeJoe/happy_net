#include "co_routinue.h"

map<int, co_schedule_t*> co_schedule::g_co_routinue_per_thread;

int co_schedule::init_env()
{
	threads.reserve(DEFAULT_THREAD_SIZE);
	sche_stack.reserve(DEFAULT_SCHEDULE_SIZE);
	
	int size = threads.capacity();
	for (int i = 0; i < size; i++)
	{
		co_routinue_t* t = new co_routinue_t();
		threads[i] = t;
		t->set_stat(FREE);
		t->set_id(i);
	}

	running = 0;
	thread_main.set_stat(RUNNING);
	sche_stack.push_back(&thread_main);
	threads.push_back(&thread_main);

	return 0;
}

int co_schedule::create(co_func_t func, void* arg)
{
	int id = 0;
	co_routinue_t* t = NULL;
	co_schedule_t* s = NULL;
	pid_t tid = gettid();
	g_co_routinue_per_thread_iter iter = g_co_routinue_per_thread.find(tid);
	if(g_co_routinue_per_thread.end() == iter)
	{
		g_co_routinue_per_thread[tid] = new co_schedule_t();
	}

	s = g_co_routinue_per_thread[tid];
	if(-1 == s->running)
	{
		s->init_env();
	}

	for (id = 0; id < s->threads.capacity(); ++id)
	{
		if(s->threads[id]->get_stat() == FREE)
		{
			break;
		}
	}

	if(id == s->threads.capacity())
		return -1;

	t = s->threads[id];
	t->set_stat(RUNNABLE);
	t->set_func(func);
	t->set_arg(arg);

	return t->get_id();
}

void co_schedule::thread_body()
{
	pid_t tid = gettid();
	co_schedule_t* s = g_co_routinue_per_thread[tid];
	co_routinue_t* t = s->sche_stack[s->running];
	//执行函数
	t->get_func()(t->get_arg());
	t->set_stat(FREE);
	//当前协程执行完毕, 切换上下文到上一个
	s->sche_stack.pop_back();
	s->running = s->running-1;
}

int co_schedule::resume(int id)
{
	pid_t tid = gettid();

	g_co_routinue_per_thread_iter iter = g_co_routinue_per_thread.find(tid);
	if(g_co_routinue_per_thread.end() == iter) return -1;
	
	co_schedule_t* s = g_co_routinue_per_thread[tid];

	if(id < 1 || id > s->threads.capacity()) return -1;

	co_routinue_t* pre = s->sche_stack[s->running];
	co_routinue_t* cur = s->threads[id];

	if(cur->get_stat() == RUNNING || cur->get_stat() == FREE) return -2;

	// cur->stat should be either 'RUNNABLE' or 'SUSPEND'
	if(cur->get_stat() == RUNNABLE)
	{
		getcontext(&cur->get_ctx());
		cur->get_ctx().uc_stack.ss_sp		= cur->get_stack();
		cur->get_ctx().uc_stack.ss_size 	= cur->get_stack_size();
		cur->get_ctx().uc_stack.ss_flags 	= 0;
		cur->get_ctx().uc_link				= &(pre->get_ctx());

		makecontext(&cur->get_ctx(), co_schedule::thread_body, 0);
	}

	pre->set_stat(SUSPEND);
	cur->set_stat(RUNNING);

	s->sche_stack.push_back(cur);

	s->running = s->running + 1;
	
	swapcontext(&pre->get_ctx(), &cur->get_ctx());

	return 0;
}

int co_schedule::yield()
{
	pid_t tid = gettid();

	g_co_routinue_per_thread_iter iter = g_co_routinue_per_thread.find(tid);
	if(g_co_routinue_per_thread.end() == iter) return -1;
	
	co_schedule_t* s = g_co_routinue_per_thread[tid];

	co_routinue_t* cur = s->sche_stack[s->running];

	if(cur == &s->thread_main) 
		return -1;

	cur->set_stat(SUSPEND);
	s->sche_stack.pop_back();
	s->running = s->running-1;
	co_routinue_t* pre = s->sche_stack[s->running];
	
	pre->set_stat(RUNNING);
	swapcontext(&cur->get_ctx(), &pre->get_ctx());

	return 0;
}

int co_schedule::release()
{
	pid_t tid = gettid();

	g_co_routinue_per_thread_iter iter = g_co_routinue_per_thread.find(tid);
	if(g_co_routinue_per_thread.end() == iter) return 0;
	
	co_schedule_t* s = g_co_routinue_per_thread[tid];
	sche_stack_iter it = s->threads.begin();
	for(; it != s->threads.end(); it++)
	{
		if((*it)->get_stat() != FREE)
		{
			return -1;
		}
	}

	it = s->threads.begin();
	for(; it != s->threads.end(); it++)
	{
		delete *it;
	}

	s->running = -1;

	delete s;

	g_co_routinue_per_thread[tid] = NULL;

	return 0;
}

int co_schedule::cur()
{
	pid_t tid = gettid();

	g_co_routinue_per_thread_iter iter = g_co_routinue_per_thread.find(tid);
	if(g_co_routinue_per_thread.end() == iter) return 0;

	co_schedule_t* s = g_co_routinue_per_thread[tid];
	co_routinue_t* cur = s->threads[s->running];
	return cur->get_id();
}

