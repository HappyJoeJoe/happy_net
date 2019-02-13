all: 
	g++ -std=c++11 -g -c fork_test.cc
	g++ -std=c++11 -g -c fork_count.cc
	
	g++ -std=c++11 -g -o ser  fork_test.o
	g++ -std=c++11 -g -o fork fork_count.o

	rm -f fork_test.o fork_count.o
clean:
	rm -f fork_test.o fork_count.o ser fork