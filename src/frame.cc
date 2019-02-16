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
#include "log.h"
#include "error.h"

/* pb */
// #include "../pb/person.pb.h"

using namespace std;

#define OPEN_FD_LIMIT   10000       /* to configure */
#define PORT 			8888        /* to configure */
#define EPOLL_SIZE 		1024        /* to configure */
#define LISTEN_SIZE 	256         /* to configure */
#define TIME_OUT 		3           /* to configure */
#define STRING_EOF      '\0'

#define READ_EVENT 		 (1 << 0)
#define WRITE_EVENT 	 (1 << 1)

#define MASTER           (1 << 0) /* master 实例 */
#define SLAVE            (1 << 1) /* slave  实例 */

// #define gettid() syscall(SYS_gettid)  

#define RETURN_CHECK(RET) \
	if(0 != RET) return RET;

#define debug_log(fmt, ...) ser.log_p->level_log(kLOG_DEBUG, fmt, ##__VA_ARGS__)
#define info_log(fmt, ...)  ser.log_p->level_log(kLOG_INFO,  fmt, ##__VA_ARGS__)
#define warn_log(fmt, ...)  ser.log_p->level_log(kLOG_WARN,  fmt, ##__VA_ARGS__)
#define err_log(fmt, ...)   ser.log_p->level_log(kLOG_ERR,   fmt, ##__VA_ARGS__)


typedef class log        	log_t;
typedef class buffer        buffer_t;
typedef struct task_s       task_t;
typedef struct connection_s connection_t;
typedef int (*event_handler)(connection_t *);
typedef void (*timer_func)(void *);

typedef list<task_t>		timer_task_queue_t;
typedef list<task_t>		io_task_queue_t;
typedef list<task_t>		accept_task_queue_t;
typedef vector<struct  epoll_event>    epoll_event_vec_t;
typedef map<uint64_t,  list<task_t>>   timer_queue_t;
typedef map<long long, connection_t*>  client_dict_t; //智能指针

typedef struct task_s
{
	void* handler;
	void* arg;
	int   type;
} task_t;

typedef struct event_s
{
	event_handler handler;
	void* arg;
	int   mask;
} event_t;

typedef struct ip_addr
{
	string  ip;
	int     port;
} ip_addr;

/* 事件循环体 */
typedef struct cycle_s
{
	int   efd;           /* epoll fd */
	int   lfd;           /* listen fd，暂时放在cycle_t里，之后放在server里 */
	unsigned long long      next_client_id;    /* 生成客户端id */
	client_dict_t           cli_dict;          /* client 字典，前期是map，后期演化成vector */
	timer_queue_t 			timer_queue;       /* 定时器存储队列 */
	io_task_queue_t 		io_task_queue;     /* IO网络事件任务队列 */
	accept_task_queue_t 	accept_task_queue; /* accept事件任务队列 */
	epoll_event_vec_t       ee_vec;            /* 激活时间vec */
	int stop:1;          /* 是否停止 */
} cycle_t;

/* 客户端请求完整请求体 */
typedef struct request_s
{
	connection_t* c;
	void* header;
	void* body;
} request_t;

/* 客户端链接 */
typedef struct connection_s
{
	long long id;            /* 客户端id */
	int fd;			         /* 套接字 */
	int mask;                /* 事件类型掩码 */
	
	ip_addr local_addr;      /* 本地地址 */
	ip_addr peer_addr;       /* 对端地址 */

	buffer_t in_buf;         /* 网络包读缓冲区 */
	buffer_t out_buf;        /* 网络包写缓冲区 */

	event_t rev;	         /* 读事件 */
	event_t wev;	         /* 写事件 */

	cycle_t* cycle_p;        /* 对应的事件循环结构体 */

	int active:1; 	         /* 链接是否已加入epoll队列中 */
	int accept:1; 	         /* 是否用于监听套接字 */
	int ready:1;  	         /* 第一次建立链接是否有开始数据，没有则不建立请求体 */
	int stop:1;              /* 删除连接用到的标记位 */
} connection_t;

typedef struct server
{
	pid_t     pid;           /* 进程pid */
	int       port;          /* 绑定端口 */
	int       open_fd_limit; /* 进程打开fd最大数，默认值 10000 */
	char*     conf_path;     /* 配置文件 */
	char*     log_path;      /* 日志文件 */
	log_t*    log_p;         /* 日志，智能指针 */
	cycle_t*  cycle_p;       /* 事件循环结构体，智能指针 */
	char*     bind_addr[16]; /* 端口绑定的本地IP列表 */
	int*      bind_count;    /* 实际绑定本地IP列表数( <= 16) */
	int       daemonize:1;   /* 是否后台模式 */
	int       type;          /* bitwise, server类型，eg: master or slave */
	int       unix_fd;       /* UNIX 域套接字 */
	char*     unix_path;     /* 路径 */
	int       cpu_num;       /* cpu核数 */
} server;

server ser;
cycle_s cycle;

int set_fd_noblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;

	int ret = fcntl(fd, F_SETFL, flags);
	if(0 != ret)
	{
		info_log("set fd noblock failed, errno msg:[%s]\n", strerror(errno));
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
		info_log("set fd cloexec failed, errno msg:[%s]\n", strerror(errno));
	}

	return 0;
}

/* 1. so_reuseaddr 允许处于[time_wait]的socket重复可用
 * 2. 允许同一端口绑定多个实例，只要每个实例绑定本地ip地址不同即可 */
int set_fd_reuseaddr(int fd)
{
	int yes = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&yes, sizeof(yes)) == -1)
    {
    	info_log("set reuseaddr failed, errno msg:[%s]\n", strerror(errno));
        return -1;
    }

	return 0;
}

int set_fd_noblock_and_cloexec(int fd)
{
	if(set_fd_noblock(fd) != 0 ||
		set_fd_cloexec(fd) != 0)
	{
		info_log("set fd noblock & cloexec failed, errno msg:[%s]\n", strerror(errno));
		return -1;
	}

	return 0;
}

int read_handler(connection_t* c);
int write_handler(connection_t* c);

int empty_handler(connection_t* c)
{
	info_log("empty_handler\n");
	return 0;
}

/* 添加epoll事件，mask为bitwise，可以兼容 READ_EVENT，WRITE_EVENT  */
int epoll_add_event(connection_t* c, int ep_fd, int sock_fd, int mask)
{
	int ret     = 0;
	int op      = c->active ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
	struct epoll_event ee = {0};

	mask |= c->mask;
	if(mask & READ_EVENT)  ee.events |= EPOLLIN | EPOLLET | EPOLLRDHUP;
	if(mask & WRITE_EVENT) ee.events |= EPOLLOUT;
	ee.data.ptr = (void *) c;

	ret = epoll_ctl(ep_fd, op, sock_fd, &ee);
	if(0 != ret) 
	{
		info_log("method:[%s] line:[%d] | errno msg:%s \n", 
			__func__, __LINE__, strerror(errno));
		return -1;
	}

	c->active = 1;

	return 0;
}

int epoll_delete_event(connection_t* c, int ep_fd, int sock_fd, int del_mask)
{
	int ret = 0;
	int op  = EPOLL_CTL_DEL;
	struct epoll_event ee = {0};

	del_mask |= c->mask;
	if(del_mask & READ_EVENT)  ee.events |= EPOLLIN | EPOLLET | EPOLLRDHUP;
	if(del_mask & WRITE_EVENT) ee.events |= EPOLLOUT;

	ret = epoll_ctl(ep_fd, op, sock_fd, &ee);
	if(0 != ret)
	{
		info_log("method:[%s] line:[%d] | errno msg:%s \n", 
			__func__, __LINE__, strerror(errno));
		return -1;
	}

	if(del_mask & READ_EVENT & WRITE_EVENT)
	{
		c->active = 0;	
	}

	return 0;
}

int epoll_add_listen(connection_t* c, int ep_fd, int listen_fd)
{
	int ret = 0;
	struct epoll_event ee = {0};
	ee.events = EPOLLIN | EPOLLRDHUP;
	ee.data.ptr = c;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, listen_fd, &ee);
	if(0 != ret)
	{
		info_log("method:[%s] line:[%d] | errno msg:%s \n", 
			__func__, __LINE__, strerror(errno));
		return -1;
	}

	return 0;
}

int epoll_del_listen(connection_t* c, int ep_fd, int listen_fd)
{
	int ret = 0;
	struct epoll_event ee = {0};
	ee.events = EPOLLIN | EPOLLRDHUP;
	ee.data.ptr = c;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_DEL, listen_fd, &ee);
	if(0 != ret)
	{
		info_log("method:[%s] line:[%d] | errno msg:%s \n", 
			__func__, __LINE__, strerror(errno));
		return -1;
	}

	return 0;
}

int accept_handler(connection_t* lc)
{
	/* 如果是监听事件，优先处理 */
	int ret = 0;
	cycle_t* cycle_p = lc->cycle_p;
	event_t* p_rev   = &lc->rev;
	int efd 	     = cycle_p->efd;

	struct sockaddr_in ca;
	socklen_t len = sizeof(ca);

	int fd = 0;
	while((fd = accept4(cycle_p->lfd, (sockaddr *)&ca, &len, SOCK_NONBLOCK)) > 0)
	{
		// info_log("loc:[%s] line:[%d]  accept fd:%d\n", __func__, __LINE__, fd);
		connection_t* p_conn = new connection_t();
		p_conn->id           = cycle_p->next_client_id++;
		p_conn->fd 			 = fd;
		p_conn->rev.handler  = read_handler;
		p_conn->rev.arg 	 = p_conn;
		p_conn->wev.handler  = empty_handler;
		p_conn->wev.arg 	 = p_conn;
		p_conn->cycle_p      = cycle_p;

		ret = set_fd_reuseaddr(fd);
		if(0 != ret)
		{
			info_log("listen fd set reuseaddr failed, errno msg:%s\n", strerror(errno));
			return -1;
		}

		epoll_add_event(p_conn, efd, p_conn->fd, READ_EVENT);

		cycle_p->cli_dict[p_conn->id] = p_conn;
	}

	return 0;
}

/* 连接第一次创建时，设置的回调函数，因为第一次连接成功后，没有数据，则不初始化链接 */
int init_request()
{
	return 0;
}

/* 一个稍微复杂的过程 */
int free_connection(connection_t* c)
{
	cycle_t* cycle_p = c->cycle_p;
	int efd 	     = cycle_p->efd;
	int fd           = c->fd;
	int id           = c->fd;

	cycle_p->cli_dict.erase(id);
	epoll_delete_event(c, efd, fd, READ_EVENT|WRITE_EVENT);
	close(fd);

	//如果没有定时器，没有buf么写完的，没有buf要读的，则调用这个，否则设置stop为1
	delete c;

	return 0;
}

/* 读取到client关闭写端，而此时server端还有数据，则连接处于半关闭状态 */
int half_close(int fd)
{
	int ret = 0;
	ret = shutdown(fd, SHUT_WR);
	if(-1 == ret)
	{
		info_log("shutdown failed, errno msg:%s", strerror(errno));
		return -1;
	}

	return 0;
}

/* 解析buffer，直到解析完整请求 */
int protocol_decoder(connection_t* c, int& err)
{
	buffer_t& in_buffer = c->in_buf;
	char* eof = in_buffer.find_eof();
	if(!eof)
	{
		return -1;
	}

	int len = eof - in_buffer.read_begin();
	// person::Persion person;


	return 0;
}

/* 协议解析 */
int parse_protocol_handler(connection_t* c)
{
	int ret = 0;
	int err = 0;
	buffer_t& in_buffer = c->in_buf;
	ret = protocol_decoder(c, err);
	if(0 != ret)
	{
		if(kDECODER_AGAIN == err)
		{
			return kDECODER_AGAIN;
		}
		
		if(kDECODER_ERROR == err)
		{
			return kDECODER_ERROR;
		}
	}

	/* 解析出完整的请求 */

	return 0;
}

/* 业务处理函数 */
int client_handler(connection_t* c)
{
	buffer_t& out_buf = c->out_buf;

	const char* str = "HTTP/1.1 200 OK\r\nServer: Tengine/2.2.2\r\nDate: Tue, 17 Jul 2018 03:02:21 GMT\r\nContent-Type: text/html\r\nContent-Length: 12\r\nConnection: keep-alive\r\n\r\nhello jiabo!";
	out_buf.append_string(str);

	return 0;
}

int read_handler(connection_t* c)
{
	cycle_t* cycle_p = c->cycle_p;
	int efd 	     = cycle_p->efd;

	event_t* p_rev = &c->rev;
	buffer_t& in_buffer = c->in_buf;
	int fd = c->fd;
	int ret = 0;
	int err = 0;

	ret = in_buffer.read_buf(fd, err);
	if(ret < 0)
	{
		if(err == kBUFFER_EAGAIN)
		{
			return kBUFFER_EAGAIN;
		}
		
		/* kBUFFER_ERROR 其他错误直接释放链接 */
		info_log("buf read error, ret:[%d] errno msg:[%s]\n", ret, strerror(errno));
		free_connection(c);	

		return -1;
	}

	if(ret < 0 && err == kBUFFER_EOF)
	{
		free_connection(c);
		return 0;
	}

	ret = parse_protocol_handler(c);
	if(0 != ret) return -1;

	client_handler(c);

	buffer_t& out_buf = c->out_buf;
	// info_log("========>  str len:%lu, buf_len:%d, buf:%s\n", strlen(str), out_buf.readable_size(), out_buf.read_begin());
	ret = out_buf.write_once(fd, err);
	// info_log("write has_write:%d unwrite:%d\n", ret, out_buf.readable_size());
	if(out_buf.readable_size() > 0)
	{
		epoll_add_event(c, efd, fd, WRITE_EVENT);
		c->wev.handler = write_handler;
	}
	else
	{
		/* 如果（没有定时器，没有buf么写完的，没有buf要读的，则调用这个，否则设置stop为1）*/
		free_connection(c);
		//否则
		c->stop = 1;
	}

	return ret;
}

int write_empty_handler(connection_t* c)
{
	info_log("%s\n", "write_empty_handler");
	return 0;
}

/* 执行用户请求后，开始对客户端写数据，若写不完，则开始对写事件设置write_handler 
   如果写完以后，要置 write_handler 为 emtpy_handler，否则有可能写事件再次触发 */
int write_handler(connection_t* c)
{
	int err = 0;
	int fd = c->fd;
	buffer_t& out_buf = c->out_buf;
	
	int ret = out_buf.write_once(fd, err);
	// info_log("[%s->%s:%d] unwrite:%d\n", __FILE__, __func__, __LINE__, out_buf.readable_size());
	if(out_buf.readable_size() == 0)
	{
		c->wev.handler = write_empty_handler;
		free_connection(c);
	}

	return ret;
}

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
// 	info_log("fuck the life\n");

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
	info_log("fuck!!!!!!!!!!!\n");
}

static int work_process_cycle()
{
	int ret = 0;
	int cnt = 0;
	int64_t timer = -1;
	
	timer_queue_t& timer_queue             = cycle.timer_queue;
	io_task_queue_t& io_task_queue         = cycle.io_task_queue;
	accept_task_queue_t& accept_task_queue = cycle.accept_task_queue;
	epoll_event_vec_t& ee_vec              = cycle.ee_vec;
	timer_task_queue_t timer_task_queue;

	pid_t id = gettid();
	info_log("work process id:%d\n", id);
	/* todo 子进程搞事情 */
	int efd = epoll_create1(0);

	// cycle.lfd = lfd;
	cycle.efd = efd;

	connection_t* p_conn = new connection_t();
	p_conn->fd = cycle.lfd;
	p_conn->rev.handler = accept_handler;
	p_conn->rev.arg = p_conn;

	p_conn->wev.handler = empty_handler;
	p_conn->wev.arg = NULL;

	p_conn->cycle_p = &cycle;
	p_conn->accept = 1;

	ret = epoll_add_listen(p_conn, efd, p_conn->fd);
	if(0 != ret)
	{
		info_log("listen fd epoll_add failed\n");
		return -1;
	}

	// vector<struct epoll_event> ee_vec(EPOLL_SIZE);

	uint64_t now = get_curr_msec();

	/* 定时器逻辑，已隐去，若开启去掉注释即可 */
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
		// info_log("now:[%lu], next timer:[%lu]\n", now, timer_end->first);
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
			// info_log("timer:[%lu]\n", timer);
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

		// info_log("\n");
		/* -------------- epoll_wait -------------- */
		cnt = epoll_wait(efd, &*(ee_vec.begin()), ee_vec.size(), timer);

		// info_log("epoll cnt[%d]\n", cnt);

		now = get_curr_msec();

		if(cnt == ee_vec.size())
		{
			ee_vec.resize(ee_vec.size() * 2);
		}

		for (int i = 0; i < cnt; ++i)
		{
			struct epoll_event* ee 	= &ee_vec[i];
			uint events 		    = ee->events;
			connection_t* c 		= (connection_t *)ee->data.ptr;
			int fd 					= c->fd;

			// info_log("loc:[%s] line:[%d] fd[%d] EPOLLIN:[%d], EPOLLOUT:[%d], EPOLLRDHUP:[%d]\n", 
			// 	__func__, __LINE__, c->fd, events & EPOLLIN, events & EPOLLOUT, events & EPOLLRDHUP);

			if(events & EPOLLRDHUP)
			{
				free_connection(c);
				continue;
			}

			if(events & EPOLLIN)
			{
				// info_log("读事件\n");
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
				// info_log("写事件\n");
				/* 加入写事件队列 */
				task_t task;
				task.handler = (void *)c->wev.handler;
				task.arg = (void *)c;
				io_task_queue.push_back(task);
			}
		}
	}

	return 0;
}

static int master_process_cycle()
{
	pid_t id = gettid();
	info_log("master process id:%d\n", id);
	// close(lfd);
	// info_log("master close lfd\n");
	/* todo master搞事情 */
	while(1)
	{
		sleep(3);
		// info_log("master_process_cycle %d\n", id);
	}
	
	return 0;
}

/* 安装信号 handler */
static void init_signal()
{

}

static void init_log()
{
	ser.log_p = new log_t();
	ser.log_p->set_log_path("ser.log");
}

static int init_ser()
{
	int ret  = 0;
	pid_t id = gettid();

	init_log();
	init_signal();

	cycle.next_client_id = 1;
	cycle.ee_vec.resize(EPOLL_SIZE); /* 调节size */
	cycle.lfd = socket(AF_INET, SOCK_STREAM, 0);

	ret = set_fd_reuseaddr(cycle.lfd);
	if(0 != ret)
	{
		info_log("listen fd set reuseaddr failed, errno msg:%s\n", strerror(errno));
		return -1;
	}

    ret = set_fd_noblock(cycle.lfd);
	if(0 != ret)
	{
		info_log("set lfd[%d] failed\n", cycle.lfd);
	}

	struct sockaddr_in addr;

	bzero(&(addr.sin_zero), 8);

   	addr.sin_family 		= AF_INET;
	addr.sin_port 			= htons(PORT);
	addr.sin_addr.s_addr   	= INADDR_ANY;
	
	ret = bind(cycle.lfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	if(0 != ret)
	{
		info_log("process[%d] bind lfd[%d] failed, errno msg:[%s]\n", id, cycle.lfd, strerror(errno));
		exit(ret);
	}

	info_log("process[%d] bind lfd[%d] sucess\n", id, cycle.lfd);

	ret = listen(cycle.lfd, LISTEN_SIZE);
	if(0 != ret)
	{
		info_log("process[%d] listen lfd[%d] failed\n", id, cycle.lfd);
		exit(ret);
	}

	info_log("process[%d] listen lfd[%d] sucess\n", id, cycle.lfd);
	return 0;
}

int daemonize()
{
	int fd;
	
	if(0 != fork()) exit(0); /* parent exit */
	setsid(); /* create a new session */

	/* /dev/null是一个无底洞，丢弃一切写入的任何东西，读取它会返回一个EOF */
	if((fd = open("/dev/null", O_RDWR, 0)) != -1)
	{
		/* dup2 为 /dev/null 指定的描述符fd创建副本，并由newfd指定编号
		 * 以下说明，fd 由标准输入，标准输出，标准错误输出三个 fd 指定其副本编号，这意味着之后的输入输出都将丢进"垃圾桶"里
		 * 调用成功后，STDIN_FILENO，STDOUT_FILENO，STDERR_FILENO都指向了 /dev/null */
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO) close(fd);
	}
}

int main(int argc, char* argv[])
{
	int ret = init_ser();
	RETURN_CHECK(ret);

	int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
	info_log("cpu_num=%d\n", cpu_num);

	/* 为方便调试work进程，暂时先把work进程逻辑放在main流程上 */
	// for (int i = 0; i < cpu_num; ++i)
	// {
	// 	pid_t pid = fork();
	// 	switch(pid)
	// 	{
	// 		case -1:
	// 			info_log("fork error!\n");
	// 			break;
	// 		case 0:
	// 			/* 子进程 */
	// 			work_process_cycle();
	// 			return 0;
	// 		default:
	// 			break;
	// 	}
	// }

	/* 主进程 */
	// master_process_cycle();

	/* 后台进程 */
	// daemonize();
	
	work_process_cycle();
	
	return 0;
}