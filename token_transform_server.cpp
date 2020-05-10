#include "token_transform_server.hpp"

#include "token_transforms.hpp"

#include <sodium.h>

namespace epione {

TokenTransformServer TokenTransformServer::Create(const string& k) {
  if (k.size() != SCALAR_SIZE) {
    throw std::invalid_argument("Incorrect size for k");
  }
  return TokenTransformServer(k);
}

TokenTransformServer::TokenTransformServer(const string& k)
    : k_(k) {
}


Status TokenTransformServer::Blind(ServerContext* context,
  const TokenSet* input_tokens, TokenSet* output_tokens) {

  // TODO: minimum number of tokens?
  // check that all tokens are the correct size
  for (const auto& t : input_tokens->token()) {
    if (!is_valid_element(t)) {
      return Status(grpc::INVALID_ARGUMENT, "Invalid token");
    }
  }

  // TODO: there must be a more efficient way of doing this
  std::vector<string> tokens(input_tokens->token().begin(),
    input_tokens->token().end());
  while (!tokens.empty()) {
    auto i = randombytes_uniform(tokens.size());
    output_tokens->add_token(blind_element(tokens[i], k_));
    tokens.erase(tokens.begin() + i);
  }
  return Status::OK;
}

}  // namespace epione