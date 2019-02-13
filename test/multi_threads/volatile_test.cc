#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

using namespace std;

volatile unsigned int a = 0;

/* 汇编
.L3:
    cmpl    $99999, -4(%rbp)
    ja  .L2
    movl    a(%rip), %eax
    addl    $1, %eax
    movl    %eax, a(%rip)
    addl    $1, -4(%rbp)
    jmp .L3 
    */
void* thread_func(void* arg)
{
	for (uint i = 0; i < 100000; ++i)
	{
		a++;
		// __sync_fetch_and_add(&a,1);
	}
}

int main(int argc, char const *argv[])
{
	int c = argc > 1 ? atoi(argv[1]) : 10;
	for (int i = 0; i < 10; ++i)
	{
		pthread_t tid[c];
		for (int i = 0; i < c; ++i)
		{
			pthread_create(&tid[i], NULL, thread_func, NULL);
		}
		for (int i = 0; i < c; ++i)
		{
			pthread_join(tid[i], NULL);
		}	
	}

	cout << a << endl;

	return 0;
}