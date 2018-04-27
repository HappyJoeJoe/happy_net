all: 
	g++ -std=c++11 -g -c co_thread.cc epoll.cc hook.cc
	g++ -std=c++11 -g -o ser frame.cc co_thread.o epoll.o hook.o -ldl

clean:
	rm -f co_thread.o epoll.o hook.o frame.o