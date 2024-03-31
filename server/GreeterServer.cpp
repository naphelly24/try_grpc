#include <grpcpp/grpcpp.h>

#include <iostream>

#include "protos/hello.grpc.pb.h"
#include "protos/hello.pb.h"

using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class GreetServer final : public Greeter::Service {
 public:
  ~GreetServer() {
    if (server_) {
      server_->Shutdown();
    }
  }

  grpc::Status SayHello(grpc::ServerContext* context,
                        const HelloRequest* request,
                        HelloReply* reply) override {
    std::cout << "Received: " << request->name() << std::endl;
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return grpc::Status::OK;
  }

  /**
   * receive a request and send multiple responses in stream
   */
  grpc::Status SayHelloServerStream(
      grpc::ServerContext* context, const HelloRequest* request,
      grpc::ServerWriter<HelloReply>* writer) override {
    std::cout << "Received: " << request->name() << std::endl;
    HelloReply reply;
    for (size_t i = 0; i < 10; i++) {
      std::string prefix("Hello " + std::to_string(i + 1) + ": ");
      reply.set_message(prefix + request->name());
      writer->Write(reply);
    }
    return grpc::Status::OK;
  }

  /**
   * receive multiple requests in stream and send a single response
   */
  grpc::Status SayHelloClientStream(grpc::ServerContext* context,
                                    grpc::ServerReader<HelloRequest>* reader,
                                    HelloReply* reply) override {
    HelloRequest request;
    std::stringstream ss;
    while (reader->Read(&request)) {
      ss << request.name() << ", ";
    }
    reply->set_message("Hello " + ss.str());
    return grpc::Status::OK;
  }

  /**
   * receive multiple requests in stream and send multiple responses in stream
   */
  grpc::Status SayHelloBidiStream(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<HelloReply, HelloRequest>* stream) override {
    HelloRequest request;
    HelloReply reply;
    std::string prefix("Hello ");
    while (stream->Read(&request)) {
      reply.set_message(prefix + request.name());
      stream->Write(reply);
    }
    return grpc::Status::OK;
  }

  void Run() {
    std::cout << "initiating server" << std::endl;
    GreetServer service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:9999", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    server_ = builder.BuildAndStart();
    std::cout << "Server listening on 0.0.0.0:9999" << std::endl;

    server_->Wait();
  }

 private:
  std::unique_ptr<grpc::Server> server_;
};

int main(int, char**) {
  GreetServer service;
  service.Run();
  return 0;
}