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
#include "comm_basic.pb.h"

#define PORT 			8888
#define IP 				"127.0.0.1"
#define CLRF			"\r\n"

typedef class buffer    buffer_t;
using namespace common;

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
	{
		int err;
		string str;

		comm_request req;
		req.mutable_head()->set_ver(1);
		req.mutable_head()->set_cmd(1);
		req.mutable_head()->set_sub_cmd(1);

		char buf[1024] = {0};
		scanf("%s", buf);

		req.set_body(buf);
		req.SerializeToString(&str);

		buffer_t query_buf;
		query_buf.append_string(str.c_str());
		query_buf.append_string(CLRF);
		// query_buf.append_string("GET / HTTP/1.0\r\nHost: localhost:8888\r\nUser-Agent: ApacheBench/2.3\r\nAccept: */*\r\n\r\n");
		ret = query_buf.write_buf(fd, err);
		
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
			common::comm_response resp;
			resp.ParseFromString(buf);
			resp_buf.read_len(strlen(CLRF));

			printf("---------- read ----------\n");
			printf("%s\n", resp.ShortDebugString().c_str());
		}
		else if(0 == ret)
		{
			printf("0 == ret\n");
		}
		else /* -1 == ret */
		{
			printf("-1 == ret\n");
		}
	}

	close(fd);

	return 0;
}