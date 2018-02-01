#include <unistd.h>
#include <stdio.h>

int process = 1;

static void work_process_cycle()
{
	printf("work process %d\n", process);
	sleep(10000);
}


int main(int argc, char* argv[])
{
	for (int i = 0; i < 2; ++i)
	{
		pid_t pid = fork();
		switch(pid)
		{
			case 0:
				work_process_cycle();
				break;
			default:
				break;
		}
		process++;
	}
	printf("master process\n");

	return 0;
}