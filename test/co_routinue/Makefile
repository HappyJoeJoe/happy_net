all: 
	g++ -std=c++11 -g -c ../../src/co_routinue.cc 
	g++ -std=c++11 -g -o co_routinue_test co_routinue_test.cc co_routinue.o -lpthread
	rm -f co_routinue.o
	
clean:
	rm -f co_routinue.o co_routinue_test