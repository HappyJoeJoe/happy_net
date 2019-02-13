#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	int n = argc > 1 ? atoi(argv[1]) : 3;
	// default done!
	// default done!
	// default done!
	// 父进程 4055
	// 子进程 4056
	// 子进程 4058
	// 子进程 4057
	{

		/* 第一次fork之后，父进程进入default分支打印 'default' 后break跳出 switch 打印“done”，而子进程进入case 0执行while循环，父进程则继续第二次fork
		 * 第二次fork之后，第一次fork的父进程重复第一次fork的逻辑
		 * 。。。。 
		 * 执行 n 次之后
		 * 只有一个父进程是第一次的父进程，其他子进程都是该父进程fork出的亲兄弟 */
		for (int32_t i = 0; i < n; ++i)
		{
			pid_t pid = fork();
			switch(pid)
			{
				case -1:
					printf("fork failed\n");
					break;
				case 0:
					
					printf("子进程 %d\n", getpid()); // 子进程
					while(1)
					{
						sleep(3);
					}
					return 0;
				default:
					printf("default ");
					break;
			}
			printf("done!\n");
		}

		printf("父进程 %d\n", getpid());
		// 主进程
		while(1)
		{
			sleep(3);
		}
	}
	
	


	/*
	{
		for (int i = 0; i < n; ++i)
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