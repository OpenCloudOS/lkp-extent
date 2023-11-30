mkdir -p ./cli
protoc --go_out=./cli --go_opt=paths=source_relative --go-grpc_opt=paths=source_relative --go-grpc_out=require_unimplemented_servers=false:./cli cli.proto

mkdir -p ./monitor
protoc --go_out=./monitor --go_opt=paths=source_relative --go-grpc_opt=paths=source_relative --go-grpc_out=require_unimplemented_servers=false:./monitor monitor.proto

mkdir -p ./sched
protoc --go_out=./sched --go_opt=paths=source_relative --go-grpc_opt=paths=source_relative --go-grpc_out=require_unimplemented_servers=false:./sched sched.proto