#include <grpcpp/grpcpp.h>

#include <iostream>
#include <thread>

#include "protos/hello.grpc.pb.h"
#include "protos/hello.pb.h"

using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

/**
 * a fresh CallData is always created to handle a new RPC call.
 * 1. HandleRpcs() blocks on cq_->Next(&tag, &ok)
 * 2. When receiving client gRPC call, gRPC fills in context, request and put the event to CompletionQueue.
 * 3. HandleRpcs() then gets the event from CompletionQueue, then calls Proceed() to handle the event, 
 * the status is PROCESS now, server will first prepare a new CallData for future RPC call, 
 * then server will do the API logic and mark status as FINISH.
 * 4. gRPC will transmission the response back to client, and put the event back to CompletionQueue.
 * 5. HandleRpcs() gets the event, and calls Proceed(), now the status is FINISH, the CallData will be deleted.
*/
class AsyncGreeterServer final : public Greeter::Service {
 public:
  ~AsyncGreeterServer() {
    if (server_) {
      server_->Shutdown();
    }
    if (cq_) {
      cq_->Shutdown();
    }
  }

  void Run() {
    std::string server_address("0.0.0.0:50051");

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    // completion queue for asynchronous communication
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
  }

 private:
  class CallData {
   public:
    CallData(Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      Proceed();
    }

    /**
     * handle the lifecycle of a single RPC call in a non-blocking manner
     */
    void Proceed() {
      if (status_ == CREATE) {
        status_ = PROCESS;
        // `this` is a pointer to current CallData, it is used as the tag to identify the specific RPC call.
        // Tell gRPC to process req when it comes in. No req for now.
        service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this);
      } else if (status_ == PROCESS) {
        // spawn a new CallData instance to serve new RPC call.
        new CallData(service_, cq_);
        std::string prefix("Hello ");
        reply_.set_message(prefix + request_.name());
        status_ = FINISH;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        responder_.Finish(reply_, grpc::Status::OK, this);
      } else {
        GPR_ASSERT(status_ == FINISH);
        delete this;
      }
    }

   private:
    Greeter::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    HelloRequest request_;
    HelloReply reply_;
    grpc::ServerAsyncResponseWriter<HelloReply> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
  };

  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    void* tag; // uniquely identifies a request.
    bool ok;
    while (true) {
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  std::unique_ptr<grpc::ServerCompletionQueue> cq_;
  Greeter::AsyncService service_;
  std::unique_ptr<grpc::Server> server_;
};

int main(int, char**) {
  AsyncGreeterServer service;
  service.Run();
  return 0;
}