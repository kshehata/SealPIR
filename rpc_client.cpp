
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "pir.grpc.pb.h"

using std::cout;
using std::endl;
using std::string;
using std::make_unique;
using std::move;
using std::unique_ptr;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace pir {

class PIRRPCClient {
 public:
  PIRRPCClient(std::shared_ptr<Channel> channel)
      : stub_(PIRService::NewStub(channel)) {}

  // Get PIR Parameters from Server
  unique_ptr<PIRParameters> GetParams() {
    PIRParametersRequest request;
    auto params = make_unique<PIRParameters>();

    ClientContext context;
    Status status = stub_->GetParams(&context, request, params.get());

    // Act upon its status.
    if (status.ok()) {
      return move(params);
    } else {
      cout << "Rquest params RPC FAILED: " << status.error_code()
      << ": " << status.error_message() << endl;
      return nullptr;
    }
  }

 private:
  std::unique_ptr<PIRService::Stub> stub_;
};

}  // namespace pir

void usage() {
  cout << "Usage: client [file] [target]" << endl;
  cout << "If [file] is specified, database values are loaded from there." << endl;
  cout << "If [target] is not specified, localhost is used." << endl;
}

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  string target_str = "localhost:50051";
  string db_filename = "database.pb";

  if (argc > 3) {
    usage();
    return 1;
  }
  if (argc > 2) {
    target_str = argv[2];
  }
  if (argc > 1) {
    db_filename = argv[1];
  }

  pir::PIRRPCClient rpc_client(grpc::CreateChannel(
      target_str, grpc::InsecureChannelCredentials()));
  auto params = rpc_client.GetParams();
  if (params != nullptr) {
    cout << "Received params: " << endl;
    params->PrintDebugString();
  }

  return 0;
}
