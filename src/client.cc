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
#include <stdlib.h>
#include <string>

#include "buffer.h"
#include "comm_basic.pb.h"

using namespace std;
using namespace common;

#define PORT 			8888
#define IP 				"127.0.0.1"
#define CLRF			"\r\n"

typedef class buffer    buffer_t;
void* client_handler(void* arg);
uint64_t get_curr_msec();


void test_conn_timeout()
{
	for(int i = 0; i < 1000; i++)
	{
		int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		struct sockaddr_in addr;
	   	addr.sin_family 		= AF_INET;
		addr.sin_port 			= htons(PORT);
		addr.sin_addr.s_addr   	= inet_addr(IP);

		int ret = connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
		if(0 != ret)
		{
			printf("connect failed ret[%d]\n", ret);
			_exit(ret);
		}
	}
}

void test_concurrence(char const *argv[])
{
	uint64_t total = 0;

	uint64_t th = 5;
	if(argv[1] != NULL)
	{
		th = stoull(argv[1]);
	}

	uint64_t num = 5;
	// if(argv[2] != NULL)
	// {
	// 	num = stoull(argv[2]);
	// }

	pthread_t ths[th];
	for(int i = 0; i < th; i++)
	{
		pthread_create(&ths[i], NULL, client_handler, (void *)&num);
	}
	for(int i = 0; i < th; i++)
	{
		void* retval;
		pthread_join(ths[i], &retval);
		total += *(static_cast<uint64_t *>(retval));
		delete (uint64_t *)retval;
	}

	printf("take %lu msec, done with %lu requests\n", total, th*num);
}

void* client_handler(void* arg)
{
	uint64_t begin = get_curr_msec();

	int* num = static_cast<int *>(arg);
	for(int i = 0; i < *num; i++)
	{
		int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		struct sockaddr_in addr;
	   	addr.sin_family 		= AF_INET;
		addr.sin_port 			= htons(PORT);
		addr.sin_addr.s_addr   	= inet_addr(IP);

		int ret = connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
		if(0 != ret)
		{
			printf("connect failed ret[%d]\n", ret);
			_exit(ret);
		}

		int err;
		string str;

		comm_request req;
		req.mutable_head()->set_ver(1);
		req.mutable_head()->set_cmd(1);
		req.mutable_head()->set_sub_cmd(1);
		string msg = "hello " + to_string(i);

		req.set_body(msg);
		req.SerializeToString(&str);

		buffer_t query_buf;
		query_buf.append_string(str);
		query_buf.append_string(CLRF);
		ret = query_buf.write_buf(fd, err);
		
		buffer_t resp_buf;
		if((ret = resp_buf.read_buf(fd, err)) > 0)
		{
			char* eof = resp_buf.find_eof(CLRF);
			if(!eof)
			{
				exit(-1);
			}
			
			string str(resp_buf.read_begin(), ret);
			common::comm_response resp;
			resp.ParseFromString(str);
			resp_buf.read_len(strlen(CLRF));
			resp.PrintDebugString();
		}
		else if(0 == ret)
		{
			printf("0 == ret\n");
		}
		else /* -1 == ret */
		{
			printf("-1 == ret\n");
		}

		close(fd);
	}

	uint64_t* retval = new uint64_t;

	*retval = get_curr_msec() - begin;

	return (void *)retval;
}


int main(int argc, char const *argv[])
{
	/* 检测连接后, 定时踢掉连接 */
	// test_conn_timeout();

	/* 测试多线程并发请求服务的性能 */
	test_concurrence(argv);

	return 0;
}


uint64_t get_curr_msec()
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