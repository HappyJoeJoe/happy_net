#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#define CPU_NUM 2

#define RETURN_CHECK(RET) \
	if(0 != RET) return RET;

int32_t process;

static int32_t work_process_cycle()
{
	pid_t id = gettid();
	printf("work process id:%d\n", id);
	//todo 子进程搞事情
	return 0;
}

static int32_t master_process_cycle()
{
	pid_t id = gettid();
	printf("master process id:%d\n", id);
	//todo master搞事情
	return 0;
}

static int32_t Init()
{
	return 0;
}

int32_t main(int32_t argc, char* argv[])
{
	int32_t ret = Init();
	RETURN_CHECK(ret);

	for (int32_t i = 0; i < CPU_NUM; ++i)
	{
		pid_t pid = fork();
		switch(pid)
		{
			case -1:
				printf("fork error!\n");
				break;
			case 0:
				//子进程
				work_process_cycle();
				return 0;
			default:
				break;
		}
	}

	//主进程
	master_process_cycle();
	
	return 0;
}