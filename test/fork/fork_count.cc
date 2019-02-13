#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main()
{

	/*
	{
		for (int32_t i = 0; i < 3; ++i)
		{
			pid_t pid = fork();
			switch(pid)
			{
				case -1:
					
					break;
				case 0:
					
					printf("子进程 %d\n", getpid()); // 子进程
					while(1)
					{
						sleep(3);
					}
					return 0;
				default:
					break;
			}
		}

		printf("父进程 %d\n", getpid());
		// 主进程
		while(1)
		{
			sleep(3);
		}
	}
	*/


	/*
	{
		for (int i = 0; i < 3; ++i)
		{
			pid_t pid = fork();
			if(0 == pid) //子进程
			{
				printf("子进程 %d\n", getpid());
				while(1)
				{
					sleep(3);
				}
			}
			else if(pid > 0) //父进程
			{
				break;
			}

		}

		printf("父进程 %d\n", getpid());
		while(1)
		{
			sleep(1);
		}	
	}
	*/
	

	return 0;
}