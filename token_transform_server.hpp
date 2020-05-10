#ifndef SEALPIR_TOKEN_TRANSFORM_SERVER_H
#define SEALPIR_TOKEN_TRANSFORM_SERVER_H value

#include <string>

#include <grpcpp/grpcpp.h>

#include "token_transforms.grpc.pb.h"

namespace epione {

using std::string;

using grpc::ServerContext;
using grpc::Status;

// Implementation of the TokenTransform server. Allows clients to get tokens
// transformed without knowing the value of the exponent k and without revealing
// the value of the tokens themselves.
//
// Clients should choose a random exponent, r, and then blind all of their
// tokens themselves before sending to this service. Afterwards, clients unblind
// tokens using the inverse of r.
class TokenTransformServer final : public TokenBlindingService::Service {
public:
  ~TokenTransformServer(){}

  Status Blind(ServerContext* context, const TokenSet* input_tokens, 
    TokenSet* output_tokens);

  // Create a server with the given k. Key must be correct length and not all
  // zero, or invalid_argument is thrown.
  static TokenTransformServer Create(const string& k);

private:
  string k_;

  // Since k will likely be a server configuration item, make sure to provide
  // it. For testing or other cases, can be random.
  TokenTransformServer(const string& k);
};

}  // namespace epione

#endif  // SEALPIR_TOKEN_TRANSFORM_SERVER_H
