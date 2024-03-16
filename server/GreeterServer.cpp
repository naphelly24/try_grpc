#include <iostream>
#include <grpcpp/grpcpp.h>
#include "protos/hello.grpc.pb.h"
#include "protos/hello.pb.h"

using Status = grpc::Status;
using ServerContext = grpc::ServerContext;
using HelloRequest = helloworld::HelloRequest;
using HelloReply = helloworld::HelloReply;

class GreetServiceImpl final : public helloworld::Greeter::Service {
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloReply* reply) override {
    std::cout << "Received: " << request->name() << std::endl;
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
  }
};

int main(int, char**) {
  std::cout << "initiating server" << std::endl;
  GreetServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:9999", grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on 0.0.0.0:9999" << std::endl;

  server->Wait();

  return 0;
}