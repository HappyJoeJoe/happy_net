all:
	#g++ -std=c++11 -g -c co_thread.cc 
	#g++ -std=c++11 -g -c buffer.cc 
	#g++ -std=c++11 -g -c log.cc 
	#g++ -std=c++11 -g -c person.pb.cc
	#g++ -std=c++11 -g -o ser frame.cc co_thread.o buffer.o log.o person.pb.o -ldl -lprotobuf
	#g++ -std=c++11 -g -o cli client.c buffer.o person.pb.o -ldl -lprotobuf
	
	g++ -std=c++11 -g -c *.cc
	g++ -std=c++11 -g -o ser frame.cc co_thread.o buffer.o log.o person.pb.o -ldl -lprotobuf
	g++ -std=c++11 -g -o cli client.c buffer.o person.pb.o -ldl -lprotobuf

	rm -f *.o

clean:
	rm -f *.o ser cli