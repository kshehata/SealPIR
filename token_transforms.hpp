#ifndef SEALPIR_TOKEN_TRANSFORMS_H
#define SEALPIR_TOKEN_TRANSFORMS_H value

#include <string>

using std::string;

// Convenience function to generate a random scalar value.
string generate_random_scalar();

// Produce a group element that corresponds to an input value.
string value_to_group(const string& value);

// Blind a token by exponentiation to the power r.
string blind_token(const string& token, const string& r);

// Remove the blinding applied to a group element. Note that you don't get the
// token value back, but the group element it maps to.
string unblind_element(const string& v, const string& r);

#endif  // SEALPIR_TOKEN_TRANSFORMS_H
