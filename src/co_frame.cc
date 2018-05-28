#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>  
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <endian.h>

#include "co_thread.h"

#define CPU_NUM 4
#define PORT 	8888
#define IP 		"127.0.0.1"

// #define gettid() syscall(SYS_gettid)  

#define RETURN_CHECK(RET) \
	if(0 != RET) return RET;

int32_t process;

static int32_t work_process_cycle()
{
	int ret = 0;
	pid_t id = gettid();
	printf("work process id:%d\n", id);
	//todo 子进程搞事情
	int efd = epoll_create1(0);

	int lfd = socket(AF_INET, SOCK_STREAM, 0);

   	struct sockaddr_in addr;
   	addr->sin_family 		= AF_INET;
	addr->sin_port 			= htons(PORT);
	addr.sin_addr.s_addr   	= inet_addr(IP);
	bzero(&(addr.sin_zero), 8);         
	bind(lfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));

	ret = listen(lfd, 1024);
	if(0 != ret)
	{
		printf("listen lfd failed\n");
		exit(ret);
	}

	struct epoll_event* ee = {
		EPOLLIN | EPOLLRDHUP,
		lfd,
	};
	ret = epoll_ctl(efd, EPOLL_CTL_ADD, lfd, ee);

	printf("listen lfd sucess, lfd:[%d], ret:[%d]\n", lfd, ret);

	struct epoll_event* p_ee = (struct epoll_event *)malloc((struct epoll_event) * 1024);

	while(1)
	{
		//处理网络 I/O 的读写事件
		for_each(ite it:io_task_queue)
		{
			it->handler();
		}

		//定时器事件
		for_each(ite it:timer_task_queue)
		{
			it->handler();
		}

		uint64_t now = time(NULL);

		if(!定时器事件队列为空)
		{
			timer = timer_task_queue::top() - now;
		}
		else
		{
			timer = -1;
		}

		int cnt = epoll_wait(efd, p_ee, 1024, timer);

		for (int i = 0; i < cnt; ++i)
		{
			struct epoll_event* ee = p_ee[i];
			uint32_t events = ee->events;
			int fd = ee->data.fd;

			if(fd == lfd)
			{
				//如果是监听事件，优先处理
				struct sockaddr_in cli_addr;    
				socklen_t len = sizeof(cli_addr);    
				int fd = accept(lfd, (sockaddr *)&cli_addr, &len);
				struct epoll_event* ee = {
					EPOLLIN | EPOLLRDHUP | EPOLLOUT,
					fd,
				};
				ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, ee);
				if(0 != ret)
				{
					
				}
			}
			else
			{
				if(events | EPOLLIN | EPOLLRDHUP)
				{
					//加入读事件队列
				}
				else if(events | EPOLLOUT)
				{
					//加入写事件队列
				}
			}
		}

		for_each(ite it:timer_task_queue)
		{

		}
	}
	return 0;
}

static int32_t master_process_cycle()
{
	pid_t id = gettid();
	printf("master process id:%d\n", id);
	//todo master搞事情
	while(1)
	{
		sleep(3);
		printf("master_process_cycle %d\n", id);
	}
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