all: 
	g++ -std=c++11 -g -c volatile_test.cc
	g++ -std=c++11 -g -o ser volatile_test.o -lpthread

	rm -f volatile_test.o
clean:
	rm -f volatile_test.o ser