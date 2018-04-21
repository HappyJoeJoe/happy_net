#include "hook.h"

void *malloc(size_t size)
{
	static int num = 0;
	static p_malloc_t my_malloc = 0;

	num++;
	
	if(my_malloc == 0)
	{
		my_malloc = (p_malloc_t)dlsym(RTLD_NEXT, "malloc");
	}

	char* p = my_malloc(size);

	printf("malloc num: %d\n", num); //禁止使用printf，不然会有惊喜发生 ^_^
	// fprintf(stderr, "malloc(%lu) = %p\n", size, p);

	return p;
}




int main()
{
	int* p = malloc(sizeof(int));
	*p = 4;
	free(p);
	return 0;
}