all: 
	g++ -std=c++11 -g -c ../../src/co_thread.cc 
	g++ -std=c++11 -g -o co_thread_test co_thread_test.cc co_thread.o -lpthread

clean:
	rm -f co_thread.o