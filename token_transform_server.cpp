#include "token_transform_server.hpp"

#include "token_transforms.hpp"

#include <sodium.h>

namespace epione {

TokenTransformServer::TokenTransformServer(const string& k)
    : k_(k) {
  // TODO: validate k here. Problem is that we can't throw. Can we?
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