/* linux header */
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
#include <sys/time.h>

/* c header */
#include <vector>
#include <list>
#include <map>
#include <algorithm>

/* user defined header */
#include "co_thread.h"

using namespace std;

#define CPU_NUM 		1
#define PORT 			8888
#define EPOLL_SIZE 		1024
#define LISTEN_SIZE 	1024
// #define IP 				"172.18.185.251"
#define IP 				"0.0.0.0"

#define READ_EVENT 		 EPOLLIN | EPOLLET | EPOLLRDHUP
#define WRITE_EVENT 	 EPOLLOUT

// #define gettid() syscall(SYS_gettid)  

#define RETURN_CHECK(RET) \
	if(0 != RET) return RET;


typedef struct task_s 			task_t;
typedef struct event_s 			event_t;
typedef struct connection_s 	connection_t;
typedef struct cycle_s 			cycle_t;
typedef int (*event_handler)(connection_t *);

int32_t process;
int lfd;

struct event_s
{
	event_handler handler;
	void* arg;
	char buf[1024];
};

struct connection_s
{
	int fd;
	int active;
	int end;
	event_t rev;
	event_t wev;
	cycle_t* cycle;
};

struct task_s
{
	void* handler;
	void* arg;
};

struct cycle_s
{
	int efd;
	int lfd;
};

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

int read_handler(connection_t* c);
int write_handler(connection_t* c);

int empty_handler(connection_t* c)
{
	printf("empty_handler\n");
	return 0;
}

void update(connection_t* c, int event_type)
{
	int ret = 0;
	int ope = 0;
	int fd = c->fd;
	int efd = c->cycle->efd;
	int events = 0;
	struct epoll_event ee;

	if(c->active)
	{
		ope = EPOLL_CTL_MOD;
	}
	else
	{
		ope = EPOLL_CTL_ADD;
		c->active = 1;
	}

	if(event_type == READ_EVENT)
	{
		if(ope == EPOLL_CTL_ADD)
		{
			events = event_type;
		}
		else
		{
			events = event_type | EPOLLOUT;
		}
	}
	else
	{
		if(ope == EPOLL_CTL_ADD)
		{
			events = EPOLLOUT;
		}
		else
		{
			events = event_type | EPOLLIN | EPOLLET | EPOLLRDHUP;
		}
	}

	ee.events |= events;
	ee.data.ptr = (void *) c;
	
	ret = epoll_ctl(efd, ope, fd, &ee);
}

int accept_handler(connection_t* lc)
{
	//如果是监听事件，优先处理
	int ret = 0;
	cycle_t* cycle = lc->cycle;
	event_t* p_rev = &lc->rev;
	// int lfd 	   = lc->fd;
	int efd 	   = cycle->efd;

	struct sockaddr_in ca;    
	socklen_t len = sizeof(ca);

	int fd = 0;
	while((fd = accept4(lfd, (sockaddr *)&ca, &len, SOCK_NONBLOCK)) > 0)
	{
		connection_t* p_conn = new connection_t();
		p_conn->fd 			 = fd;
		p_conn->rev.handler  = read_handler;
		p_conn->rev.arg 	 = p_conn;
		p_conn->wev.handler  = write_handler;
		p_conn->wev.arg 	 = p_conn;

		p_conn->cycle = lc->cycle;

		struct epoll_event ee = 
		{
			EPOLLIN | EPOLLET | EPOLLRDHUP,
			(void *)p_conn
		};

		p_conn->active = 1;
		ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ee);
		if(0 != ret)
		{
			
		}
		
		printf("lfd[%d] accept fd[%d] sucess\n", lfd, fd);
	}

	printf("accept_handler complete\n");

	return 0;
}

int read_handler(connection_t* c)
{
	event_t* p_rev = &c->rev;
	int fd = c->fd;
	char buf[1024] = {0};
	size_t size_buf = 12;
	int ret = 0;
	int num = 0;

	while(1) 
	{
		ret = recv(fd, buf, 1024, 0);
		if(ret > 0)
		{
			num += ret;
			printf("recv[%d] buf:[%s] size:[%d]\n", fd, buf, ret);
			// return ret;
			break;
		}
		if(ret == 0)
		{
			printf("eof\n");
			return 0;
		}
		if(errno == EAGAIN | errno == EWOULDBLOCK)
		{
			return errno;
		}
	}

	ret = send(fd, buf, 12, 0);
	if(ret > 0)
	{
		char tmp[16] = {0};
		memcpy(tmp, buf, ret);
		printf("send[%d] buf:[%s], ret:[%d]\n", fd, tmp, ret);
	}

	if(ret < num)
	{
		printf("EAGAIN:[%d]\n", EAGAIN);
		update(c, READ_EVENT);
		c->wev.handler = write_handler;
		memcpy(c->wev.buf, buf+ret, num-ret);

		return ret;
	}

	return num;
}

int write_handler(connection_t* c)
{
	event_t* p_wev = &c->wev;
	int fd = c->fd;
	char* buf = p_wev->buf;
	size_t size_buf = 1024;
	int ret = 0;
	int num = 0;

	while(1)
	{
		ret = send(fd, buf, size_buf, 0);
		if(ret > 0)
		{
			printf("send() buf:[%s]\n", buf);
			return ret;
		}
		
		if(0 == ret)
		{
			printf("send() size = 0\n");
			return ret;
		}

		if (ret == EAGAIN || ret == EINTR) 
		{
	        printf("send() not ready\n");
	        if (ret == EAGAIN) {
	    		return EAGAIN;
	    	}
	    }
	    else
	    {
	    	return ret;
	    }
	}
}

typedef list<task_t>		timer_task_queue_t;
typedef list<task_t>		io_task_queue_t;
typedef map<uint64_t, list<task_t>>	timer_queue_t;

static uint64_t get_curr_msec()
{
	uint64_t msec;
	struct timeval tv;

	int ret = gettimeofday(&tv, 0);
	if(0 != ret)
	{
		return -1;
	}

    msec = tv.tv_sec + tv.tv_usec / 1000;

	return msec;
}

static int32_t work_process_cycle()
{
	int ret = 0;
	int cnt = 0;
	int64_t timer = -1;

	cycle_s cycle;
	
	timer_task_queue_t 	timer_task_queue;
	io_task_queue_t 	io_task_queue;
	timer_queue_t 		timer_queue;

	pid_t id = gettid();
	printf("work process id:%d\n", id);
	//todo 子进程搞事情
	int efd = epoll_create(1024);

	// cycle.lfd = lfd;
	cycle.efd = efd;

	connection_t* p_conn = new connection_t();
	p_conn->fd = lfd;
	p_conn->rev.handler = accept_handler;
	p_conn->rev.arg = p_conn;

	p_conn->wev.handler = empty_handler;
	p_conn->wev.arg = NULL;

	p_conn->cycle = &cycle;

	struct epoll_event ee = 
	{
		EPOLLIN,
		(void *)p_conn
	};

	ret = epoll_ctl(efd, EPOLL_CTL_ADD, lfd, &ee);
	if(0 != ret)
	{
		
	}

	vector<struct epoll_event> ee_vec(EPOLL_SIZE);

	uint64_t now = get_curr_msec();

	while(1)
	{
		timer_queue_t::iterator timer_end = timer_queue.lower_bound(now);
		if(now >= timer_end->first)
		{
			for_each(timer_queue.begin(), timer_end, [& timer_task_queue](pair<const uint64_t, list<task_t>>& p)
			{
				timer_task_queue.splice(timer_task_queue.end(), p.second);
			});
			
			timer_queue.erase(timer_queue.begin(), timer_end);
		}

		/* -------------- 定时器事件 -------------- */
		for_each(timer_task_queue.begin(), timer_task_queue.end(), [](task_t& t) 
		{
			((event_handler)t.handler)((connection_t *)t.arg);
		});

		timer_task_queue.clear();

		if(timer_queue.size() > 0)
		{
			timer_queue_t::iterator it = timer_queue.begin();
			timer = it->first - now;
		}
		else
		{
			timer = -1;
		}

		/* -------------- 网络 I/O 的读写事件 -------------- */
		for_each(io_task_queue.begin(), io_task_queue.end(), [](task_t& t) 
		{
			((event_handler)t.handler)((connection_t *)t.arg);
		});

		io_task_queue.clear();

		printf("\n");
		/* -------------- epoll_wait -------------- */
		cnt = epoll_wait(efd, &*(ee_vec.begin()), ee_vec.size(), timer);

		printf("epoll cnt[%d]\n", cnt);

		now = time(NULL);

		if(cnt == ee_vec.size())
		{
			ee_vec.resize(ee_vec.size() * 2);
		}

		for (int i = 0; i < cnt; ++i)
		{
			struct epoll_event* ee 	= &ee_vec[i];
			uint32_t events 		= ee->events;
			connection_t* c 		= (connection_t *)ee->data.ptr;

			printf("fd[%d] events[%d], events & EPOLLIN:[%d], events & EPOLLOUT:[%d], events & EPOLLRDHUP:[%d]\n", 
				c->fd, events, events & EPOLLIN, events & EPOLLOUT, events & EPOLLRDHUP);

			if(events & EPOLLRDHUP)
			{
				ret = epoll_ctl(efd, EPOLL_CTL_DEL, c->fd, ee);
				if(0 != ret)
				{
					printf("epoll_ctl del err, fd[%d], ret[%d]\n", c->fd, ret);
				}
				close(c->fd);
				printf("客户端fd:[%d] 断联\n", c->fd);
				continue;
			}

			if(events & EPOLLIN)
			{
				printf("读事件\n");
				//加入读事件队列
				task_t task;
				task.handler = (void *)c->rev.handler;
				task.arg = (void *)c;
				io_task_queue.push_back(task);
			}
			
			if(events & EPOLLOUT)
			{
				printf("写事件\n");
				//加入写事件队列
				task_t task;
				task.handler = (void *)c->wev.handler;
				task.arg = (void *)c;
				io_task_queue.push_back(task);
			}
		}
	}
	return 0;
}

static int32_t master_process_cycle()
{
	pid_t id = gettid();
	printf("master process id:%d\n", id);
	// close(lfd);
	// printf("master close lfd\n");
	//todo master搞事情
	while(1)
	{
		sleep(3);
		// printf("master_process_cycle %d\n", id);
	}
	return 0;
}

static int32_t Init()
{
	pid_t id = gettid();

	lfd = socket(AF_INET, SOCK_STREAM, 0);

	int ret = set_fd_noblock(lfd);
	if(0 != ret)
	{
		printf("set lfd[%d] failed\n", lfd);
	}


	struct sockaddr_in addr;

	bzero(&(addr.sin_zero), 8);

   	addr.sin_family 		= AF_INET;
	addr.sin_port 			= htons(PORT);
	// addr.sin_addr.s_addr   	= inet_addr(IP);
	addr.sin_addr.s_addr   	= INADDR_ANY;
	
	ret = bind(lfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	if(0 != ret)
	{
		printf("process[%d] bind lfd[%d] failed\n", id, lfd);
		exit(ret);
	}

	printf("process[%d] bind lfd[%d] sucess\n", id, lfd);

	ret = listen(lfd, LISTEN_SIZE);
	if(0 != ret)
	{
		printf("process[%d] listen lfd[%d] failed\n", id, lfd);
		exit(ret);
	}

	printf("process[%d] listen lfd[%d] sucess\n", id, lfd);
	return 0;
}

int32_t main(int32_t argc, char* argv[])
{
	int32_t ret = Init();
	RETURN_CHECK(ret);

	// for (int32_t i = 0; i < CPU_NUM; ++i)
	// {
	// 	pid_t pid = fork();
	// 	switch(pid)
	// 	{
	// 		case -1:
	// 			printf("fork error!\n");
	// 			break;
	// 		case 0:
	// 			//子进程
	// 			work_process_cycle();
	// 			return 0;
	// 		default:
	// 			break;
	// 	}
	// }

	// //主进程
	// master_process_cycle();

	work_process_cycle();
	
	return 0;
}