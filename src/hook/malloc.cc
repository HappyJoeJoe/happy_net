#include "malloc.h"

void *malloc(size_t size)
{
	static int num = 0;
	static p_malloc_t my_malloc = 0;

	num++;
	
	if(my_malloc == 0)
	{
		my_malloc = (p_malloc_t)dlsym(RTLD_NEXT, "malloc");
	}

	char* p = (char*)my_malloc(size);

	fprintf(stderr, "malloc(%lu) = %p\n", size, p);

	return p;
}

