#ifndef __TIMER_H__
#define __TIMER_H__

#include <map>

class timer
{
public:
	explicit timer(int fd) : timer_fd:(fd)
	{
		
	}
	
	~timer() 
	{
		
	}

private:
	int timer_fd;
};


#endif