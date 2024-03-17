#include <grpcpp/grpcpp.h>

#include <iostream>
#include <thread>

#include "protos/hello.grpc.pb.h"
#include "protos/hello.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class GreeterClient {
 private:
  std::unique_ptr<Greeter::Stub> stub_;

 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  std::string SayHello(const std::string& user) {
    HelloRequest request;
    request.set_name(user);

    ClientContext context;
    HelloReply reply;
    Status status = stub_->SayHello(&context, request, &reply);

    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string SayHelloServerStream(const std::string& user) {
    HelloRequest request;
    request.set_name(user);
    HelloReply reply;
    ClientContext context;
    std::unique_ptr<grpc::ClientReader<HelloReply>> reader(
        stub_->SayHelloServerStream(&context, request));
    std::stringstream ss;
    while (reader->Read(&reply)) {
      ss << reply.message() << std::endl;
    }
    Status status = reader->Finish();
    if (status.ok()) {
      return ss.str();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string SayHelloClientStream(const std::string& user) {
    HelloRequest request;
    HelloReply reply;
    ClientContext context;

    std::unique_ptr<grpc::ClientWriter<HelloRequest>> writer(
        stub_->SayHelloClientStream(&context, &reply));

    for (size_t i = 0; i < 5; i++) {
      request.set_name(user + " " + std::to_string(i + 1));
      if (!writer->Write(request)) {
        break;
      }

      // rest for a while
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    writer->WritesDone();

    Status status = writer->Finish();
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string SayHelloBidiStream(const std::string& user) {
    HelloReply reply;
    ClientContext context;
    std::shared_ptr<grpc::ClientReaderWriter<HelloRequest, HelloReply>> stream(
        stub_->SayHelloBidiStream(&context));

    std::thread writer([stream, user]() {
      HelloRequest request;

      for (size_t i = 0; i < 5; i++) {
        request.set_name(user + " " + std::to_string(i + 1));
        if (!stream->Write(request)) {
          break;
        }
        // rest for a while
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
      stream->WritesDone();
    });

    std::stringstream ss;
    /**
     * stream->Read(&reply) will block if there are no messages currently available but the stream is still open. 
     * It will only return false when the stream is closed or the RPC is cancelled.
    */
    while (stream->Read(&reply)) {
      std::cout << "Received: " << reply.message() << std::endl;
      ss << reply.message() << std::endl;
    }
    /**
     * The writer.join() is used to ensure that the main thread waits for the writer thread to finish before proceeding. 
     * However, removing writer.join() doesn't necessarily mean that you will miss data in the while (stream->Read(&reply)) loop.
     * In this case, the while (stream->Read(&reply)) loop is able to handle all the data even without writer.join() because of the way how gRPC works.
     * gRPC uses HTTP/2, which supports bidirectional streams. This means that the client can start reading as soon as it has started writing. 
     * The writer thread writes a message and then sleeps for a second. During this time, the main thread is free to read messages from the stream.
     * When the writer thread finishes writing all messages, it calls stream->WritesDone(). 
     * This signals to the server that no more messages will be written. 
     * These responses can still be read by the client after stream->WritesDone() is called. 
     * So, even without writer.join(), the main thread can still read all responses because it continues reading until stream->Read(&reply) returns false, 
     * which happens when all responses have been read and the server has closed the stream. 
     * However, this behavior depends on the specific timing and behavior of gRPC. 
     * In general, it's a good idea to ensure that all writing is done before you call stream->Finish().
    */
    writer.join();
    
    Status status = stream->Finish();
    if (status.ok()) {
      return ss.str();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }
};

void usage(const char* name) {
  std::cout << "usage: " << name << " <action>" << std::endl;
  std::cout << "\taction: 1: SayHello" << std::endl
            << "\t\t2: SayHelloServerStream" << std::endl
            << "\t\t3: SayHelloClientStream" << std::endl
            << "\t\t4: SayHelloBidirectionalStream" << std::endl;
}

int main(int argc, char const* argv[]) {
  GreeterClient greeter(grpc::CreateChannel(
      "127.0.0.1:9999", grpc::InsecureChannelCredentials()));
  std::string user("Mr. World");
  if (argc != 2) {
    usage(argv[0]);
  }

  if (strcmp(argv[1], "1") == 0) {
    std::string reply = greeter.SayHello(user);
    std::cout << "Unary mode, Greeter received: " << reply << std::endl;
  } else if (strcmp(argv[1], "2") == 0) {
    std::string replies = greeter.SayHelloServerStream(user);
    std::cout << "Server stream mode, Greeter received: " << std::endl
              << replies << std::endl;
  } else if (strcmp(argv[1], "3") == 0) {
    std::string reply = greeter.SayHelloClientStream(user);
    std::cout << "Client stream mode, Greeter received: " << reply << std::endl;
  } else if (strcmp(argv[1], "4") == 0) {
    std::string replies = greeter.SayHelloBidiStream(user);
    std::cout << "Bidi stream mode, Greeter received stream: " << std::endl
              << replies << std::endl;
  } else {
    usage(argv[0]);
  }

  return 0;
}
