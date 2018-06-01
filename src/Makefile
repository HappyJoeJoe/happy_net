all: 
	g++ -std=c++11 -g -c co_thread.cc 
	g++ -std=c++11 -g -o ser frame.cc co_thread.o -ldl

	g++ -std=c++11 -g -o cli client.c

clean:
	rm -f co_thread.o frame.o ser cli