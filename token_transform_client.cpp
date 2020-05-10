#include "token_transform_client.hpp"

#include "token_transforms.hpp"

#include <iostream>

namespace epione {

using std::cout;
using std::endl;

using grpc::ClientContext;
using grpc::Status;

vector<string> TokenTransformClient::BlindTokens(vector<string> tokens) {
  TokenSet sent_tokens;
  TokenSet received_tokens;

  // Use a fresh value of r for every request, unless we've been given r
  if (r_.size() <= 0) {
    r_ = generate_random_scalar();
  }

  sent_tokens.mutable_token()->Reserve(tokens.size());
  for (const auto& t : tokens) {
    sent_tokens.add_token(blind_token(t, r_));
  }

  ClientContext context;
  Status status = stub_->Blind(&context, sent_tokens, &received_tokens);

  if (!status.ok()) {
    // TODO should probably throw an exception or do something more here.
    cout << "Token blinding RPC FAILED: " << status.error_code()
    << ": " << status.error_message() << endl;
    return {};
  }

  // TODO: check that results from server are valid?

  vector<string> results;
  results.reserve(received_tokens.token().size());
  for (const auto& t : received_tokens.token()) {
    results.push_back(unblind_element(t, r_));
  }

  r_ = "";
  return results;
}

}  // namespace epione