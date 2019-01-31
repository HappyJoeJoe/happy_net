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
#include <string.h>

/* user defined header */
#include "co_thread.h"
#include "buffer.h"
#include "error.h"

using namespace std;

#define CPU_NUM 		1
#define PORT 			8888
#define EPOLL_SIZE 		1024
#define LISTEN_SIZE 	256
#define TIME_OUT 		3
#define STRING_EOF      '\0'

#define READ_EVENT 		 EPOLLIN | EPOLLET | EPOLLRDHUP
#define WRITE_EVENT 	 EPOLLOUT

// #define gettid() syscall(SYS_gettid)  

#define RETURN_CHECK(RET) \
	if(0 != RET) return RET;

typedef struct connection_s connection_t;
typedef class buffer            buffer_t;
typedef int (*event_handler)(connection_t *);
typedef void (*timer_func)(void *);

int read_example_handler(connection_t* c);

int32_t process;
int lfd;

typedef struct event_s
{
	event_handler handler;
	void* arg;
	int timer:1;	//是否设置了定时器
} event_t;

typedef struct ip_addr
{
	string ip;
	int32_t port;
} ip_addr;

typedef struct task_s
{
	void* handler;
	void* arg;
} task_t;

typedef struct cycle_s
{
	int efd;
	int lfd;
} cycle_t;

typedef struct connection_s
{
	int fd;			    //套接字
	
	ip_addr local_addr; //本地地址
	ip_addr peer_addr;  //对端地址

	buffer_t in_buf;    //网络包读缓冲区
	buffer_t out_buf;   //网络包写缓冲区

	event_t rev;	    //读事件
	event_t wev;	    //写事件

	cycle_t* cycle;     //对应的事件循环结构体

	int active:1; 	    //链接是否已加入epoll队列中
	int accept:1; 	    //是否用于监听套接字
	int ready:1;  	    //第一次建立链接是否有开始数据，没有则不建立请求体
} connection_t;

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
	struct epoll_event ee = {0};
	
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
			// events = event_type | EPOLLIN | EPOLLRDHUP;
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
	int efd 	   = cycle->efd;

	struct sockaddr_in ca;
	socklen_t len = sizeof(ca);

	int fd = 0;
	while((fd = accept4(lfd, (sockaddr *)&ca, &len, SOCK_NONBLOCK)) > 0)
	{
		printf("loc:[%s] line:[%d]  accept fd:%d\n", __func__, __LINE__, fd);
		connection_t* p_conn = new connection_t();
		p_conn->fd 			 = fd;
		p_conn->rev.handler  = read_example_handler;
		p_conn->rev.arg 	 = p_conn;
		p_conn->wev.handler  = empty_handler;
		p_conn->wev.arg 	 = p_conn;

		p_conn->cycle = lc->cycle;

		struct epoll_event ee = 
		{
			EPOLLIN | EPOLLET | EPOLLRDHUP,
			(void *)p_conn
		};

		update(p_conn, READ_EVENT);
		// p_conn->active = 1;
		// ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ee);
		// if(0 != ret)
		// {
			
		// }
		

		// printf("lfd[%d] accept fd[%d] sucess\n", lfd, fd);
	}

	// printf("accept_handler complete\n");

	return 0;
}

/* 连接第一次创建时，设置的回调函数，因为第一次连接成功后，没有数据，则不初始化链接 */
int init_request()
{
	
}

int read_handler(connection_t* c)
{
	cycle_t* cycle = c->cycle;
	int efd 	   = cycle->efd;

	event_t* p_rev = &c->rev;
	int fd = c->fd;
	char buf[128] = {0};
	int ret = 0;
	int num = 0;

	while(1) 
	{
		ret = recv(fd, buf, 128, 0);
		if(ret > 0)
		{
			num += ret;
			printf("recv fd:[%d] buf:[%s] size:[%d]\n", fd, buf, ret);
			// return ret;
			break;
		}
		if(ret == 0)
		{
			printf("eof\n");
			return 0;
		}
		if(errno == EAGAIN || errno == EWOULDBLOCK)
		{
			return errno;
		}
	}

	const char* str = "HTTP/1.1 200 OK\r\nServer: Tengine/2.2.2\r\nDate: Tue, 17 Jul 2018 03:02:21 GMT\r\nContent-Type: text/html\r\nContent-Length: 12\r\nConnection: keep-alive\r\n\r\nhello jiabo!";
	int size = strlen(str);

	ret = send(fd, str, size, 0);
	if(ret == size)
	{
		printf("send fd:[%d] buf:[%s], ret:[%d]\n", fd, str, ret);
		free_connection(c);
		return ret;
	}

	if(ret < size)
	{
		printf("EAGAIN:[%d]\n", EAGAIN);
		update(c, READ_EVENT);
		c->wev.handler = write_handler; //install write handler

		return ret;
	}

	return num;
}

int free_connection(connection_t* c)
{
	int fd = c->fd;
	struct epoll_event ee;
	ee.events = 0;
	ee.data.ptr = NULL;
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ee);
	close(fd);
	delete c;

	return 0;	
}

/* 协议解析 */
int parse_protocol_handler(connection_t* c)
{
	buffer_t& in_buffer = c->in_buf;
	int ret = 0;
	int buffer_len = in_buffer.readable_size();
	char buf[buffer_len + 1];
	buf[buffer_len] = '\0';
	in_buffer.get_string(buffer_len, buf);
	// char* line = strchr(buf, '/');
	printf("loc:[%s] line:[%d]  ans:%s\n", __func__, __LINE__, buf);

	return 0;
}

int client_handler(connection_t* c)
{
	return 0;
}

int read_example_handler(connection_t* c)
{
	cycle_t* cycle = c->cycle;
	int efd 	   = cycle->efd;

	event_t* p_rev = &c->rev;
	buffer_t& in_buffer = c->in_buf;
	int fd = c->fd;
	int ret = 0;
	int err = 0;

	ret = in_buffer.read_buf(fd, err);
	if(ret < 0)
	{
		printf("%s\n", "close client connection");
		return -1;
	}

	// char buf[ret+1];
	// buf[ret] = STRING_EOF;

	// in_buffer.get_string(ret, buf);
	// printf("方法:%s 行号:%d ==> recv fd:[%d] buf:[%s] size:[%d], readable_size:%d\n", 
	// 	__func__, __LINE__, fd, buf, ret, in_buffer.readable_size());

	ret = parse_protocol_handler(c);
	if(ret != 0) return -1;

	client_handler(c);

	const char* ptr =
	 	"HTTP/1.1 200 OK\r\nServer: Tengine/2.2.2\r\nDate: Tue, 17 Jul 2018 03:02:21 GMT\r\nContent-Type: text/html\r\nContent-Length: 12\r\nConnection: keep-alive\r\n\r\nhello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo hello jiabo !";
	// const char* ptr = ss.str().c_str();
	int size = strlen(ptr);
	int len = size/2;
	buffer_t& out_buf = c->out_buf;

	ret = write(fd, ptr, len);
	if(ret == size)
	{
		printf("send complete:[%d]\n", ret);
		struct epoll_event ee;
		ee.events = 0;
		ee.data.ptr = NULL;
		ret = epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ee);

		close(fd);

		delete c;
		return ret;
	}

	if(ret < size)
	{
		printf("send EAGAIN\n");
		out_buf.append_string(size - ret, ptr + ret);
		update(c, READ_EVENT);
		c->wev.handler = write_handler;

		return ret;
	}

	return ret;
}

int write_empty_handler(connection_t* c)
{
	printf("%s\n", "write_empty_handler");
	return 0;
}

/* 执行用户请求后，开始对客户端写数据，若写不完，则开始对写事件设置write_handler 
   如果写完以后，要置 write_handler 为 emtpy_handler，否则有可能写事件再次出发 */
int write_handler(connection_t* c)
{
	event_t* p_wev = &c->wev;
	int fd = c->fd;
	buffer_t& out_buffer = c->out_buf;
	size_t size_buf = 1024 * 1024;
	int ret = 0;
	int num = 0;

	char* tmp = new char[size_buf];
	memset(tmp, 2, size_buf);

	while(1)
	{
		ret = write(fd, tmp, size_buf);
		if(ret > 0)
		{
			return ret;
		}
		
		if(0 == ret)
		{
			printf("write() size = 0\n");
			return ret;
		}

		if (ret == EAGAIN || ret == EINTR) 
		{
	        printf("write() not ready\n");
	        if (ret == EAGAIN) {
	    		return EAGAIN;
	    	}
	    }
	    else
	    {
	    	return ret;
	    }
	}

	c->wev.handler = write_empty_handler;

	return ret;
}



typedef list<task_t>		timer_task_queue_t;
typedef list<task_t>		io_task_queue_t;
typedef list<task_t>		accept_task_queue_t;
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

    msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return msec;
}

// void timer_func(void* arg)
// {
// 	printf("fuck the life\n");

// 	timer_queue_t& timer_queue = *static_cast<timer_queue_t*>(arg);
// 	uint64_t now = get_curr_msec();
// 	uint64_t three = now + 3000;

// 	list<task_t> p;
// 	task_t timer_task = {
// 		(void *)timer_func,
// 		&timer_queue
// 	};

// 	p.push_back(timer_task);

// 	timer_queue[three] = p;
// }

struct every_timer_arg
{
	timer_func func;
	void* arg;
	void* ctx;
	uint64_t every;
};

void every_timer_func(void* arg)
{
	struct every_timer_arg* t_arg = static_cast<struct every_timer_arg*>(arg);

	timer_func func = t_arg->func;
	void* func_arg = t_arg->arg;
	((event_handler)func)((connection_t *)func_arg);

	timer_queue_t& timer_queue = *static_cast<timer_queue_t*>(t_arg->ctx);
	
	uint64_t now = get_curr_msec();
	uint64_t when = now + t_arg->every * 1000;

	list<task_t> p;

	task_t every_timer_task = {
		(void *)every_timer_func,
		(void *)t_arg
	};

	p.push_back(every_timer_task);

	timer_queue[when] = p;
}

void add_every_timer(timer_queue_t& timer_queue, uint64_t after, uint64_t every, timer_func func, void* arg)
{
	uint64_t now = get_curr_msec();
	uint64_t when = now + after * 1000;

	list<task_t> p;

	struct every_timer_arg* t_arg = (struct every_timer_arg*)malloc(sizeof(struct every_timer_arg));
	t_arg->func = func;
	t_arg->arg = arg;
	t_arg->ctx = &timer_queue;
	t_arg->every = every;

	task_t every_timer_task = {
		(void *)every_timer_func,
		(void *)t_arg
	};

	p.push_back(every_timer_task);

	timer_queue[when] = p;
}

void add_timer_after(timer_queue_t& timer_queue, uint64_t after, timer_func func, void* arg)
{
	uint64_t now = get_curr_msec();
	uint64_t when = now + after * 1000;

	list<task_t> p;
	task_t timer_task = {
		(void *)func,
		(void *)arg
	};

	p.push_back(timer_task);

	timer_queue[when] = p;
}

int delete_timer()
{

}

void tmp_timer(void* arg)
{
	printf("fuck!!!!!!!!!!!\n");
}

static int32_t work_process_cycle()
{
	int ret = 0;
	int cnt = 0;
	int64_t timer = -1;

	cycle_s cycle;
	
	timer_task_queue_t 		timer_task_queue;
	timer_queue_t 			timer_queue;
	io_task_queue_t 		io_task_queue;
	accept_task_queue_t 	accept_task_queue;


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
	p_conn->accept = 1;

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

	// add_every_timer(timer_queue, 3, 2, tmp_timer, NULL);

	while(1)
	{
		/* -------------- accept 事件 -------------- */
		for_each(accept_task_queue.begin(), accept_task_queue.end(), [](task_t& t) 
		{
			((event_handler)t.handler)((connection_t *)t.arg);
		});

		accept_task_queue.clear();

		timer_queue_t::iterator timer_end = timer_queue.lower_bound(now);
		// printf("now:[%lu], next timer:[%lu]\n", now, timer_end->first);
		if(now >= timer_end->first)
		{
			for_each(timer_queue.begin(), timer_end, [& timer_task_queue](pair<const uint64_t, list<task_t>>& p)
			{
				timer_task_queue.splice(timer_task_queue.end(), p.second);
			});
			
			timer_queue.erase(timer_queue.begin(), timer_end);

			/* -------------- 定时器事件 -------------- */
			for_each(timer_task_queue.begin(), timer_task_queue.end(), [](task_t& t) 
			{
				((event_handler)t.handler)((connection_t *)t.arg);
			});

			timer_task_queue.clear();
		}

		if(timer_queue.size() > 0)
		{
			timer_end = timer_queue.lower_bound(now);
			timer = timer_end->first - now;
			// printf("timer:[%lu]\n", timer);
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

		// printf("\n");
		/* -------------- epoll_wait -------------- */
		cnt = epoll_wait(efd, &*(ee_vec.begin()), ee_vec.size(), timer);

		// printf("epoll cnt[%d]\n", cnt);

		now = get_curr_msec();

		if(cnt == ee_vec.size())
		{
			ee_vec.resize(ee_vec.size() * 2);
		}

		for (int i = 0; i < cnt; ++i)
		{
			struct epoll_event* ee 	= &ee_vec[i];
			uint32_t events 		= ee->events;
			connection_t* c 		= (connection_t *)ee->data.ptr;
			int fd 					= c->fd;

			printf("loc:[%s] line:[%d] fd[%d] EPOLLIN:[%d], EPOLLOUT:[%d], EPOLLRDHUP:[%d]\n", 
				__func__, __LINE__, c->fd, events & EPOLLIN, events & EPOLLOUT, events & EPOLLRDHUP);

			// if(events & EPOLLRDHUP)
			// {
			// 	ret = epoll_ctl(efd, EPOLL_CTL_DEL, c->fd, ee);
			// 	if(0 != ret)
			// 	{
			// 		printf("epoll_ctl del err, fd[%d], ret[%d]\n", c->fd, ret);
			// 	}
			// 	close(c->fd);
			// 	continue;
			// }

			if(events & EPOLLIN)
			{
				// printf("读事件\n");
				if(!c->ready)
				{

					c->ready = 1;
				}

				task_t task;
				task.handler = (void *)c->rev.handler;
				task.arg = (void *)c;

				if(c->accept)
				// if(fd == lfd)
				{
					// ((event_handler)task.handler)((connection_t *)task.arg);
					accept_task_queue.push_back(task);
				}
				else
				{
					//加入读事件队列
					io_task_queue.push_back(task);	
				}
			}
			
			if(events & EPOLLOUT)
			{
				// printf("写事件\n");
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
	int ret = 0;
	int reuseaddr = 1;
	pid_t id = gettid();

	printf("AF_INET:[%d], SOCK_STREAM:[%d]\n", AF_INET, SOCK_STREAM);

	lfd = socket(AF_INET, SOCK_STREAM, 0);

	if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &reuseaddr, sizeof(int))
        == -1)
    {
        // if (ngx_close_socket(s) == -1) {
        //     ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
        //                   ngx_close_socket_n " %V failed",
        //                   &ls[i].addr_text);
        // }

        return -1;
    }

    ret = set_fd_noblock(lfd);
	if(0 != ret)
	{
		printf("set lfd[%d] failed\n", lfd);
	}

	struct sockaddr_in addr;

	bzero(&(addr.sin_zero), 8);

   	addr.sin_family 		= AF_INET;
	addr.sin_port 			= htons(PORT);
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