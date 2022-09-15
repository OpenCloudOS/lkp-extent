ALL:lkp_extent lkp_extent_CLI

lkp_extent:lkp_extent.cc lkpServer.cc lkpServer.h lkpClient.cc lkpClient.h lkpHelper.cc lkpHelper.h lib/lkpCodec.cc lib/lkpCodec.h lib/lkpDispatcher.h lib/lkpProto.pb.cc lib/lkpProto.pb.h
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -lprotobuf -lz -std=c++11 -g

lkp_extent_CLI:lkpCommand.cc lkpHelper.cc lkpHelper.h lib/lkpCodec.cc lib/lkpCodec.h lib/lkpDispatcher.h lib/lkpProto.pb.cc lib/lkpProto.pb.h
	-g++ $^ -o ./build/$@ -lmuduo_net -lmuduo_base -lpthread -lprotobuf -lz -std=c++11 -g

clean:
	rm ./build/lkp_extent
	rm ./build/lkp_extent_CLI