#include "../../src/co_routinue.h"
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <iostream>
#include <pthread.h>
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
	printf("wahahahahahahahahahahahaha\n");
	co_resume(co_2);
	printf("World\n");
}

typedef struct test
{
	
}test;

class A
{
public:
	A()
	{
		printf("hello I'm A\n");
	}
	~A()
	{
		printf("hello I'm A~~\n");
	};
	
};

const int i = 20;

const static int m = 20;

int32_t main(int32_t argc, char* argv[])
{
	// test* s = singleton<test>::get_instance();

	int arg1 = 11;
	int arg2 = 22;
	co_1 = co_create(func1, &arg1);
	co_2 = co_create(func2, &arg2);
	co_resume(co_1);

	// co_release();
	// printf("main over\n");

	// const int j = 20;

	// const static int n = 20;

	// printf("sizeof(i):[%lu], sizeof(j):[%lu]\n", sizeof(i), sizeof(j));

	return 0;
}