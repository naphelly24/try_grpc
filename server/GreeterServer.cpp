#include <grpcpp/grpcpp.h>

#include <iostream>
#include <thread>

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

  /**
  *This means that even though SayHello is a blocking operation, it doesn't block other RPC calls from being processed.
  When multiple gRPC calls come, they are handled by different threads on the server. These threads run concurrently,
  so the SayHello operations can execute simultaneously, even though each individual operation is blocking.
  */
  grpc::Status SayHello(grpc::ServerContext* context, const HelloRequest* request, HelloReply* reply) override {
    std::cout << "In thread " << std::this_thread::get_id() << " ,received: " << request->name() << std::endl;
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return grpc::Status::OK;
  }

  /**
   * receive a request and send multiple responses in stream
   */
  grpc::Status SayHelloServerStream(grpc::ServerContext* context, const HelloRequest* request,
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
  grpc::Status SayHelloClientStream(grpc::ServerContext* context, grpc::ServerReader<HelloRequest>* reader,
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
  grpc::Status SayHelloBidiStream(grpc::ServerContext* context,
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