all: 
	g++ -std=c++11 -g -c ../../src/log.cc -ldl
	g++ -std=c++11 -g -o log_test log_test.cc log.o -ldl

	rm -f log.o

clean:
	rm -f log.o log_test