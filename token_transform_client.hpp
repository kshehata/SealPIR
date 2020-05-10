#ifndef SEALPIR_TOKEN_TRANSFORM_CLIENT_H
#define SEALPIR_TOKEN_TRANSFORM_CLIENT_H value

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "token_transforms.grpc.pb.h"

namespace epione {

using std::string;
using std::unique_ptr;
using std::vector;
using grpc::Channel;

class TokenTransformClient {
public:
  TokenTransformClient(unique_ptr<TokenBlindingService::StubInterface> stub)
      : stub_(std::move(stub)) {}

  ~TokenTransformClient(){}

  // Blind tokens by exponentiating them first with a random r value and then
  // send them to the server, where they're again exponentiated with a value
  // known only to the server. Finally, on the client end the initial r is
  // removed so that only the server-side blinding remains.
  vector<string> BlindTokens(vector<string> tokens);

  // helper mainly for testing, allows setting r value to be used to blind
  // tokens before being sent to the server. Only used once before being thrown
  // away.
  void set_r(const string& r) {
    r_ = r;
  }

private:
  unique_ptr<TokenBlindingService::StubInterface> stub_;
  string r_;
};

}  // namespace epione

#endif  // SEALPIR_TOKEN_TRANSFORM_CLIENT_H
