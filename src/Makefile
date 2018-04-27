all: 
	g++ -std=c++11 -g -c co_thread.cc co_schedule.cc epoll.cc hook.cc
	g++ -std=c++11 -g -o ser frame.cc co_thread.o co_schedule.o epoll.o hook.o -ldl

clean:
	rm -f co_thread.o co_schedule.o epoll.o hook.o frame.o