#include <grpcpp/grpcpp.h>

#include <iostream>

#include "protos/hello.grpc.pb.h"
#include "protos/hello.pb.h"

using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using helloworld::HelloReply;
using helloworld::HelloRequest;
using helloworld::Greeter;
using grpc::ClientContext;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char const* argv[]) {
  GreeterClient greeter(grpc::CreateChannel("127.0.0.1:9999", grpc::InsecureChannelCredentials()));
  std::string user("Mr. World");
  std::string reply = greeter.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;
  return 0;
}
