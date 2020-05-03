
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "pir.grpc.pb.h"
#include "pir.hpp"
#include "pir_database.hpp"
#include "pir_server.hpp"

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace pir {


// Logic and data behind the server's behavior.
class PIRServiceImpl final : public PIRService::Service {
public:

  PIRServiceImpl(unique_ptr<PIRParameters> params, unique_ptr<PIRDatabase> db) :
    params_(std::move(params)) {

    EncryptionParameters enc_params(seal::scheme_type::BFV);
    PirParams pir_params;
    gen_params(params_->items_in_db(), params_->size_per_item(),
      params_->mod_degree(), params_->logt(), params_->d(),
      enc_params, pir_params);
    pir_server_ = make_unique<PIRServer>(enc_params, pir_params);

    cout << "Setting up database" << endl;
    auto time_pre_s = high_resolution_clock::now();
    auto pir_db = gen_database(*db, enc_params, pir_params);
    auto time_pre_e = high_resolution_clock::now();
    auto time_pre_us = duration_cast<microseconds>(time_pre_e - time_pre_s).count();
    cout << "PIRServer pre-processing time: " << time_pre_us / 1000 << " ms" << endl;

    pir_server_->set_database(move(pir_db));
  }

  Status GetParams(ServerContext* context, const PIRParametersRequest* request,
                  PIRParameters* reply) override {
    cout << "sending params" << endl;
    (*reply) = (*params_);
    return Status::OK;
  }

  Status PrivateQuery(ServerContext* context, const PIRQuery* request,
                  PIRReply* result) override {

    cout << "Deserializing Galois Keys" << endl;
    seal::GaloisKeys galois_keys = deserialize_galoiskeys(request->keys());

    cout << "Deserializing query" << endl;
    PirQuery query = deserialize_pir_query(*request);

    cout << "Processing query" << endl;
    auto time_server_s = high_resolution_clock::now();
    PirReply reply = pir_server_->generate_reply(query, galois_keys);
    auto time_server_e = high_resolution_clock::now();
    auto time_server_us = duration_cast<microseconds>(time_server_e - time_server_s).count();
    cout << "PIRServer reply generation time: " << time_server_us / 1000 << " ms"
         << endl;

    serialize_ciphertexts(reply, result->mutable_reply());
    return Status::OK;
  }  

private:

  unique_ptr<PIRParameters> params_;
  unique_ptr<PIRServer> pir_server_;

};

void RunServer(const string& db_filename) {
  auto db = load_database_from_file(db_filename);
  auto params = make_unique<PIRParameters>();
  params->set_items_in_db(db->item_size());
  params->set_size_per_item(db->item(0).value().size());
  // TODO(kshehata): These should really be configurable.
  params->set_mod_degree(2048);
  params->set_logt(12);
  params->set_d(2);

  string server_address("0.0.0.0:50051");
  PIRServiceImpl service(move(params), move(db));

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << server_address << endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

}  // namespace pir

int main(int argc, char** argv) {
  cout << "Starting Server..." << endl;
  pir::RunServer("database.pb");
  cout << "All done!" << endl;

  return 0;
}