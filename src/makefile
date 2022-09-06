ALL:lkpServer lkpClient lkpCommand

lkpServer:Server.cc lkpServer.cc lkpServer.h lib/lkpCodec.cc lib/lkpCodec.h lib/lkpDispatcher.h lib/lkpProto.pb.cc lib/lkpProto.pb.h
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -lmuduo_protorpc -lprotobuf -lz -std=c++11 -g
lkpClient:lkpClient.cc lib/lkpCodec.cc lib/lkpCodec.h lib/lkpDispatcher.h lib/lkpProto.pb.cc lib/lkpProto.pb.h
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -lmuduo_protorpc -lprotobuf -lz -std=c++11 -g
lkpCommand:lkpCommand.cc lib/lkpCodec.h lib/lkpDispatcher.h lib/lkpProto.pb.cc lib/lkpProto.pb.h
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -lmuduo_protorpc -lprotobuf -lz -std=c++11 -g
