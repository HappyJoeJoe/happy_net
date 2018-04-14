#include <unistd.h>
#include <stdio.h>

#define CPU_NUM 1

int32_t process = 1;

static int32_t work_process_cycle()
{
	printf("work process %d\n", process);
	//todo 子进程搞事情
}

static int32_t master_process_cycle()
{
	//todo master搞事情
	
}

static int32_t Init()
{
	
}

int32_t main(int32_t argc, char* argv[])
{

	for (int32_t i = 0; i < CPU_NUM; ++i)
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
	
	master_process_cycle();

	return 0;
}