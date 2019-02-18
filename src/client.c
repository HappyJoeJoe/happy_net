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

#include "buffer.h"
#include "request.pb.h"

#define PORT 			8888
#define IP 				"172.18.185.251"
#define CLRF			"\r\n"

typedef class buffer    buffer_t;

int main(int argc, char const *argv[])
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

	for(int i = 0; i < 1; i++)
	// while(1)
	{
		int err;
		string str;

		common::comm_request req;
		req.mutable_head()->set_ver(1);
		req.mutable_head()->set_cmd(1);
		req.mutable_head()->set_sub_cmd(1);
		req.mutable_head()->set_err_code(100);
		req.mutable_head()->set_err_msg("wow");
		req.SerializeToString(&str);

		buffer_t query_buf;
		query_buf.append_string(str.c_str());
		query_buf.append_string(CLRF);
		// query_buf.append_string("GET / HTTP/1.0\r\nHost: localhost:8888\r\nUser-Agent: ApacheBench/2.3\r\nAccept: */*\r\n\r\n");
		ret = query_buf.write_once(fd, err);

		// sleep(3);

		buffer_t resp_buf;
		if((ret = resp_buf.read_buf(fd, err)) > 0)
		{
			char* eof = resp_buf.find_eof(CLRF);
			if(!eof)
			{
				return -1;
			}
			
			int len = eof - resp_buf.read_begin();
			char buf[len+1];
			buf[len] = '\0';
			resp_buf.get_string(len, buf);
			common::comm_request tmp;
			tmp.ParseFromString(buf);
			resp_buf.read_len(strlen(CLRF));

			printf("---------- read ----------\n");
			printf("[%s]\n", tmp.ShortDebugString().c_str());
		}
		else if(0 == ret)
		{
			printf("0 == ret\n");
		}
		else /* -1 == ret */
		{
			printf("0 == ret\n");
		}
	}

	close(fd);

	// int efd = epoll_create(1024);
	// struct epoll_event* p_ee = (struct epoll_event *)malloc(sizeof(struct epoll_event) * 1024);

	// struct epoll_event ee = 
	// {
	// 	EPOLLIN,
	// 	STDIN_FILENO
	// };

	// ret = epoll_ctl(efd, EPOLL_CTL_ADD, STDIN_FILENO, &ee);

	
	// // ret = write(fd, buf, sizeof(buf));

	// while(1)
	// {
	// 	int cnt = epoll_wait(efd, p_ee, 1024, -1);
	// 	for (int i = 0; i < cnt; ++i)
	// 	{
	// 		struct epoll_event* e = &p_ee[i];
	// 		if(e->data.fd = STDIN_FILENO)
	// 		{
	// 			char buf[128] = {0};
	// 			ret = read(STDIN_FILENO, buf, sizeof(buf));
	// 			printf("读入[%s]\n", buf);
	// 			ret = write(fd, buf, sizeof(buf));
	// 		}
	// 		// char tmp[128] = {0};
	// 		// ret = read(fd, tmp, sizeof(tmp));
	// 	 // 	printf("read tmp[%s]\n", tmp);

			
	// 	}
	// }

	return 0;
}