#include "../src/hook.h"

int main()
{
	int* p = malloc(sizeof(int));
	*p = 4;
	free(p);
	return 0;
}