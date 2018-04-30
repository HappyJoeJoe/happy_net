#include "co_thread.h"
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;

int co_1 = 0;
int co_2 = 0;
int co_3 = 0;

void func2(void* arg)
{
	int num = *(int*)arg;
	printf("yahohohohohohohohohohohoho %d\n", num);
	co_yield();
	printf("goooooooooooooooooooooogle\n");
}

void func1(void* arg)
{
	int num = *(int*)arg;
	printf("Hello %d\n", num);
	co_resume(co_2);
	printf("World\n");
}

int32_t main(int32_t argc, char* argv[])
{
	int arg1 = 11;
	int arg2 = 22;
	co_1 = co_create(func1, &arg1);
	co_2 = co_create(func2, &arg2);
	co_resume(co_1);

	// co_3 = co_create(func1, 0);
	// co_resume(co_3);
	printf("main over\n");
	
	return 0;
}