#include <unistd.h>

int process;

static void work_process_cycle()
{
	printf("work process %d\n", process++);
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
		}

	}
	printf("master process\n");

	return 0;
}