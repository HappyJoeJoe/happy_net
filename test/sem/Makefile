all: 
	g++ -std=c++11 -g -c posix_text.cc system_v_test.cc
	g++ -std=c++11 -g -o posix posix_text.o 
	g++ -std=c++11 -g -o system_v system_v_test.o

	rm -f *.o
clean:
	rm -f *.o posix system_v