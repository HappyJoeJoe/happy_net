#include "co_thread.h"

int co_schedule::init_env()
{
	get_threads().reserve(DEFAULT_THREAD_SIZE);
	get_sche_stack().reserve(DEFAULT_THREAD_SIZE);
	max_index = DEFAULT_THREAD_SIZE;
	int size = get_threads().capacity();
	for (int i = 0; i < size; i++)
	{
		co_thread_t* t = new co_thread_t();
		get_threads()[i] = t;
		t->set_stat(FREE);
	}

	set_running(0);
	get_thread_main().set_stat(RUNNING);
	get_sche_stack()[get_running()]	= &get_thread_main();
	get_threads()[get_running()]	= &get_thread_main();

	return 0;
}

int co_schedule::create(co_func_t func, void* arg)
{
	int id = 0;
	co_thread_t* t = NULL;
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();
	if(-1 == s->get_running())
	{
		s->init_env();
	}

	for (id = 0; id < s->max_index; ++id)
	{
		if(s->get_threads()[id]->get_stat() == FREE)
		{
			break;
		}
	}

	if(id == s->max_index)
		return -1;

	t = s->get_threads()[id];
	t->set_stat(RUNNABLE);
	t->set_func(func);
	t->set_arg(arg);

	return id;
}

void co_schedule::thread_body()
{
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();
	co_thread_t* t = s->get_threads()[s->get_running()];
	t->get_func()(t->get_arg());
	t->set_stat(FREE);
	s->get_sche_stack().pop_back();
	s->set_running(s->get_running()-1);
}

int co_schedule::resume(int id)
{
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();
	if(id < 1 || id > s->max_index) return -1;

	co_thread_t* pre = s->get_threads()[s->get_running()];
	co_thread_t* cur = s->get_threads()[id];

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

	s->get_sche_stack().push_back(cur);
	s->set_running(s->get_running()+1);

	swapcontext(&pre->get_ctx(), &cur->get_ctx());

	return 0;
}

int co_schedule::yield()
{
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();

	co_thread_t* cur = s->get_threads()[s->get_running()];

	if(cur == &s->get_thread_main()) 
		return -1;

	cur->set_stat(SUSPEND);
	s->get_sche_stack().pop_back();
	s->set_running(s->get_running()-1);
	co_thread_t* pre = s->get_threads()[s->get_running()];
	pre->set_stat(RUNNING);
	swapcontext(&cur->get_ctx(), &pre->get_ctx());

	return 0;
}

int co_schedule::release()
{
	co_schedule_t* s = singleton<co_schedule_t>::get_instance();
	sche_stack_ite it = s->get_threads().begin();
	for(; it != s->get_threads().end(); it++)
	{
		if((*it)->get_stat() != FREE)
		{
			return -1;
		}
	}

	it = s->get_threads().begin();
	for(; it != s->get_threads().end(); it++)
	{
		delete *it;
	}

	s->set_running(-1);

	delete s;

	return 0;
}

