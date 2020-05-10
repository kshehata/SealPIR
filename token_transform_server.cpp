#include "token_transform_server.hpp"

#include "token_transforms.hpp"

#include <sodium.h>

namespace epione {

TokenTransformServer TokenTransformServer::Create(const string& k) {
  if (k.size() != crypto_core_ristretto255_SCALARBYTES) {
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

  // TODO: there must be a more efficient way of doing this
  std::vector<string> tokens(input_tokens->token().begin(),
    input_tokens->token().end());
  while (!tokens.empty()) {
    auto i = randombytes_uniform(tokens.size());
    output_tokens->add_token(blind_token(tokens[i], k_));
    tokens.erase(tokens.begin() + i);
  }
  return Status::OK;
}

}  // namespace epione