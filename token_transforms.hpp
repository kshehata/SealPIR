#ifndef SEALPIR_TOKEN_TRANSFORMS_H
#define SEALPIR_TOKEN_TRANSFORMS_H value

#include <sodium.h>
#include <string>

using std::string;

constexpr size_t TOKEN_SIZE = 16;
constexpr size_t SCALAR_SIZE = crypto_core_ristretto255_SCALARBYTES;
constexpr size_t GROUP_ELEMENT_SIZE = crypto_core_ristretto255_BYTES;

// Convenience function to generate a random scalar value.
string generate_random_scalar();

// Produce a group element that corresponds to an input value.
string value_to_element(const string& value);

// Check whether a value is a valid group element. Returns true if it is.
bool is_valid_element(const string& p);

// Blind a group element by exponentiation to the power r.
string blind_element(const string& u, const string& r);

// Convenience function to generate a group element from a token and then blind
// it using exponentation to r.
string blind_token(const string& token, const string& r);

// Remove the blinding applied to a group element. Note that you don't get the
// token value back, but the group element it maps to.
string unblind_element(const string& v, const string& r);

#endif  // SEALPIR_TOKEN_TRANSFORMS_H
