all:
	g++ -std=c++11 -g -c *.cc
	g++ -std=c++11 -g -o ser frame.cc co_thread.o buffer.o log.o comm_basic.pb.o -ldl -lprotobuf
	g++ -std=c++11 -g -o cli client.c buffer.o comm_basic.pb.o -ldl -lprotobuf

	rm -f *.o

clean:
	rm -f *.o ser cli