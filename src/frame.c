#include <unistd.h>

int process;

static void work_process_cycle()
{
	for(;;)
	{
		printf("work process %d", process++);
		sleep(10000);
	}
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

	}
	printf("master process");
	return 0;
}