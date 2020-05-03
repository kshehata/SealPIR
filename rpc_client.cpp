
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "pir.grpc.pb.h"
#include "pir_client.hpp"
#include "pir_database.hpp"

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
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
      cout << "Request params RPC FAILED: " << status.error_code()
      << ": " << status.error_message() << endl;
      return nullptr;
    }
  }

  string GetItem(uint32_t ele_index) {
    auto params = GetParams();
    if (params == nullptr) {
      return "";
    }

    seal::EncryptionParameters enc_params(seal::scheme_type::BFV);
    PirParams pir_params;
    gen_params(params->items_in_db(), params->size_per_item(),
      params->mod_degree(), params->logt(), params->d(),
      enc_params, pir_params);

    PIRClient client(enc_params, pir_params);
    seal::GaloisKeys galois_keys = client.generate_galois_keys();

    uint64_t index = client.get_fv_index(ele_index, params->size_per_item());   // index of FV plaintext
    uint64_t offset = client.get_fv_offset(ele_index, params->size_per_item()); // offset in FV plaintext
    cout << "Client: element index = " << ele_index << " from [0, " << params->size_per_item() -1 << "]" << endl;
    cout << "Client: FV index = " << index << ", FV offset = " << offset << endl; 

    // Measure query generation
    auto time_query_s = high_resolution_clock::now();
    PirQuery query = client.generate_query(index);
    auto time_query_e = high_resolution_clock::now();
    auto time_query_us = duration_cast<microseconds>(time_query_e - time_query_s).count();
    cout << "Client: query generated" << endl;
    cout << "PIRClient query generation time: " << time_query_us / 1000 << " ms" << endl;


    PIRQuery request;
    request.set_keys(serialize_galoiskeys(galois_keys));
    serialize_pir_query(query, &request);

    PIRReply reply;
    ClientContext context;
    Status status = stub_->PrivateQuery(&context, request, &reply);

    if (!status.ok()) {
      cout << "PIR Query RPC FAILED: " << status.error_code()
      << ": " << status.error_message() << endl;
      return "";
    }

    PirReply pir_reply = deserialize_ciphertexts(reply.reply());

    auto time_decode_s = chrono::high_resolution_clock::now();
    Plaintext result = client.decode_reply(pir_reply);
    auto time_decode_e = chrono::high_resolution_clock::now();
    auto time_decode_us = duration_cast<microseconds>(time_decode_e - time_decode_s).count();
    cout << "Main: PIRClient answer decode time: " << time_decode_us / 1000 << " ms" << endl;

    // Convert from FV plaintext (polynomial) to database element at the client
    string elems(params->mod_degree() * params->logt() / 8, 0);
    coeffs_to_bytes(params->logt(), result, (uint8_t*)elems.data(),
      (params->mod_degree() * params->logt()) / 8);

    return elems.substr(offset * params->size_per_item(),
      params->size_per_item());
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

  auto db = load_database_from_file(db_filename);
  random_device rd;
  uint64_t ele_index = rd() % db->item_size(); 

  pir::PIRRPCClient rpc_client(grpc::CreateChannel(
      target_str, grpc::InsecureChannelCredentials()));
  auto item = rpc_client.GetItem(ele_index);
  if (db->item(ele_index).value() == item) {
    cout << "PIR retrieval successful!" << endl;
  } else {
    cout << "PIR retrieval failed, item doesn't match!" << endl;
  }

  return 0;
}
