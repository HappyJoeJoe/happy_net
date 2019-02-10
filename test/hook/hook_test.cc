#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>


typedef void *(*p_malloc_t)(size_t);


void *malloc(size_t size)
{
	static int num = 0;
	static p_malloc_t my_malloc = 0;

	num++;
	
	if(my_malloc == 0)
	{
		my_malloc = (p_malloc_t)dlsym(RTLD_NEXT, "malloc");
	}

	char* p = (char *)my_malloc(size);

	// printf("malloc num: %d\n", num); //禁止使用printf，不然会有惊喜发生 ^_^
	fprintf(stderr, "my malloc(%lu) = %p\n", size, p);

	return p;
}



int main()
{
	double d;
	d = 1.0;
	/*
	int* p = malloc(sizeof(int));
	*p = 4;
	free(p);
	*/
	
	printf("%s\n", "\e[1;32m显示的文字\e[0m");
	return 0;
}