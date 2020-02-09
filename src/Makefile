all:
	protoc comm_basic.proto --cpp_out=.
	g++ -std=c++11 -g -c *.cc
	g++ -std=c++11 -g -o ser frame.cc co_routinue.o global.o buffer.o log.o comm_basic.pb.o -ldl -lprotobuf -lpthread
# 	g++ -std=c++11 -g -o ser frame_timerfd.cc co_routinue.o global.o buffer.o log.o comm_basic.pb.o -ldl -lprotobuf -lpthread -lrt
	g++ -std=c++11 -g -o cli client.cc buffer.o comm_basic.pb.o -ldl -lprotobuf -lpthread

	rm -f *.o

clean:
	rm -f *.o ser cli