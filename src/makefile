ALL:Server Client OLD_CMDclient

Server:NewServer.cc lkpServer.cc lkpServer.h lkpCodec.cc lkpCodec.h lkpDispatcher.h lkpProto.pb.cc lkpProto.pb.h
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -lmuduo_protorpc -lprotobuf -lz -std=c++11 -g
Client:lkpClient.cc lkpCodec.cc lkpCodec.h lkpDispatcher.h lkpProto.pb.cc lkpProto.pb.h
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -lmuduo_protorpc -lprotobuf -lz -std=c++11 -g
%:%.cc
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -lmuduo_protorpc -lprotobuf -lz -std=c++11 -g
