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

#define PORT 			8888
#define IP 				"172.18.185.251"
#define CLRF			\r\n


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
		char buf[128] = "GET / HTTP/1.0\r\nHost: localhost:8888\r\nUser-Agent: ApacheBench/2.3\r\nAccept: */*\r\n\r\n";
		ret = write(fd, buf, sizeof(buf));
		printf("write buf[%s]\n", buf);

		// sleep(3);

		char tmp[128] = {0};
		if((ret = read(fd, tmp, sizeof(tmp))) > 0)
		{
			printf("read tmp[%s]\n", tmp);
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