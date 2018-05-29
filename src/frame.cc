#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>  
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <endian.h>
#include <fcntl.h>
#include <vector>

#include "co_thread.h"

#define CPU_NUM 		4
#define PORT 			8888
#define IP 				"127.0.0.1"
#define EPOLL_SIZE 		10240
#define LISTEN_SIZE 	1024

#define gettid() syscall(SYS_gettid)  

#define RETURN_CHECK(RET) \
	if(0 != RET) return RET;

int32_t process;

int set_fd_noblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;

	int ret = fcntl(fd, F_SETFL, flags);
	if(0 != ret)
	{

	}

	return 0;
}

int set_fd_cloexec(int fd)
{
	int flags = fcntl(fd, F_GETFD, 0);
	flags |= FD_CLOEXEC;

	int ret = fcntl(fd, F_SETFD, flags);
	if(0 != ret)
	{

	}

	return 0;
}

int set_fd_noblock_and_cloexec(int fd)
{
	if(set_fd_noblock(fd) == 0 &&
		set_fd_cloexec(fd) == 0)
	{
		return 0;
	}

	return -1;
}

typedef int (*event_handler)(connection_t *);

typedef struct event_s
{
	event_handler handler;
	void* arg;
	char buf[1024];
} event_t;

typedef struct connection_s
{
	int fd;
	event_t rev;
	event_t wev;
} connection_t;

typedef struct task_s
{
	void* handler;
	void* arg;
} task_t;


int read_handler(connection_t* c)
{
	event_t* p_rev = &c->rev;
	int fd = c->fd;
	char* buf = p_rev->buf;
	size_t size_buf = 1024;
	int ret = 0;
	while((ret = read(fd, buf, size_buf)) != EAGAIN) 
	{
		printf("read buf:[%s]\n", buf);
	}

}

int write_handler(connection_t* c)
{
	event_t* p_wev = &c->wev;
	int fd = c->fd;
	char* buf = p_wev->buf;
	size_t size_buf = 1024;
	int ret = 0;
	while((ret = write(fd, buf, size_buf)) != EAGAIN) 
	{
		printf("write buf:[%s]\n", buf);
	}
}

typedef timer_task_queue_t vector<task_t>;
typedef io_task_queue_t vector<task_t>;

static int32_t work_process_cycle()
{
	int ret = 0;
	uint64_t timer = -1;
	timer_task_queue_t timer_task_queue;
	io_task_queue_t io_task_queue;
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

	ret = listen(lfd, LISTEN_SIZE);
	if(0 != ret)
	{
		printf("listen lfd failed\n");
		exit(ret);
	}

	struct epoll_event* ee = 
	{
		EPOLLIN | EPOLLRDHUP,
		lfd,
	};
	ret = epoll_ctl(efd, EPOLL_CTL_ADD, lfd, ee);

	printf("listen lfd sucess, lfd:[%d], ret:[%d]\n", lfd, ret);

	struct epoll_event* p_ee = (struct epoll_event *)malloc((struct epoll_event) * EPOLL_SIZE);

	uint64_t now = time(NULL);

	while(1)
	{
		uint64_t top = timer_queue::top();
		uint64_t idx = top;
		if(now >= top)
		{
			for_each(timer_queue.begin(), timer_queue.end(), [](timer_task_queue_t::iterator it) 
			{
				if(now >= *it)
				{
					timer_task_queue.push(*it);
					idx++;
				}
			});

			timer_queue.remove(top, idx);
		}

		// -------------- 定时器事件 --------------
		for_each(timer_task_queue.begin(), timer_task_queue.end(), [](timer_task_queue_t::iterator it) 
		{
			it->handler(it->arg);
		});

		timer_task_queue.clear();

		if(!timer_queue.size())
		{
			timer = timer_queue::top() - now;
		}
		else
		{
			timer = -1;
		}

		// -------------- 网络 I/O 的读写事件 --------------
		for_each(io_task_queue.begin(), io_task_queue.end(), [](io_task_queue_t::iterator it) 
		{
			it->handler(it->arg);
		});

		io_task_queue.clear();

		// -------------- epoll_wait --------------
		int cnt = epoll_wait(efd, p_ee, EPOLL_SIZE, timer);

		now = time(NULL);

		for (int i = 0; i < cnt; ++i)
		{
			struct epoll_event* ee 	= p_ee[i];
			uint32_t events 		= ee->events;
			connection_t* c 		= (connection_t *)ee->data.ptr;
			int fd = c->fd;

			if(fd == lfd)
			{
				//如果是监听事件，优先处理
				struct sockaddr_in cli_addr;    
				socklen_t len = sizeof(cli_addr);    
				int fd = accept(lfd, (sockaddr *)&cli_addr, &len);

				ret = set_fd_noblock_and_cloexec(fd);
				if(0 != ret)
				{

				}

				connection_t* p_conn = new connection_t();
				p_conn->fd = fd;
				p_conn->rev.handler = read_handler;
				p_conn->rev.arg = p_conn;

				p_conn->wev.handler = write_handler;
				p_conn->wev.arg = p_conn;

				struct epoll_event* ee = 
				{
					EPOLLIN | EPOLLRDHUP | EPOLLOUT,
					p_conn,
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
					task_t task;
					task.handler = read_handler;
					task.arg = c;
					io_task_queue.push(task);
				}
				
				if(events | EPOLLOUT)
				{
					//加入写事件队列
					task_t task;
					task.handler = write_handler;
					task.arg = c;
					io_task_queue.push(task);
				}
			}
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