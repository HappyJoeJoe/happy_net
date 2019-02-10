all:
	g++ -std=c++11 -g -c co_thread.cc 
	g++ -std=c++11 -g -c buffer.cc 
	g++ -std=c++11 -g -c log.cc 
	g++ -std=c++11 -g -o ser frame.cc co_thread.o buffer.o log.o -ldl
	g++ -std=c++11 -g -o cli client.c buffer.o -ldl

	rm -f co_thread.o frame.o buffer.o log.o

clean:
	rm -f co_thread.o frame.o buffer.o log.o ser cli