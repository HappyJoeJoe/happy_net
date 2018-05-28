all: 
	g++ -std=c++11 -g -c co_thread.cc 
	g++ -std=c++11 -g -o ser frame.cc co_thread.o -ldl

clean:
	rm -f co_thread.o frame.o