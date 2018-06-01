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

#define PORT 			8888
#define IP 				"172.18.185.251"


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

	while(1)
	{
		sleep(3);
		char buf[12] = "hello world";
		ret = write(fd, buf, 12);
		char tmp[128] = {0};
		ret = read(fd, tmp, sizeof(tmp));
		printf("read tmp[%s]\n", tmp);
	}

	return 0;
}