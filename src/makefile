ALL:CMDserver CMDclient TCPclient

CMDserver:server.cc CMDserver.cc CMDserver.h
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -std=c++11 -g

%:%.cc
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -std=c++11 -g
