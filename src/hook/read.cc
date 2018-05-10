#include "read.h"

ssize_t read(int fd, void *buf, size_t count);
{
	static sys_read_t my_read = 0;
	if(my_read == 0)
	{
		my_read = (sys_read_t)dlsym(RTLD_NEXT, "read");
	}

	
}