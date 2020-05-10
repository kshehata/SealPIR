#include "token_transforms.hpp"

#include <sodium.h>

// TODO(kshehata): Need to call sodium_init() somewhere

string generate_random_scalar() {
  string r(crypto_core_ristretto255_SCALARBYTES, 0);
  crypto_core_ristretto255_scalar_random((unsigned char*)r.data());
  return r;
}

string value_to_element(const string& value) {
  unsigned char x[crypto_core_ristretto255_HASHBYTES];
  crypto_generichash(x, crypto_core_ristretto255_HASHBYTES,
    (unsigned char*)value.data(), value.size(), NULL, 0);
  string p(crypto_core_ristretto255_BYTES, 0);
  crypto_core_ristretto255_from_hash((unsigned char*)p.data(), x);
  return p;
}

bool is_valid_element(const string& p) {
  return p.size() == GROUP_ELEMENT_SIZE &&
      crypto_core_ristretto255_is_valid_point((unsigned char*)p.data());
}

string blind_element(const string& u, const string& r) {
  if (r.size() != crypto_core_ristretto255_SCALARBYTES) {
    throw std::invalid_argument("Scalar r incorrect size");
  }
  if (u.size() != crypto_core_ristretto255_BYTES) {
    throw std::invalid_argument("Group element u incorrect size");
  }
  string v(crypto_core_ristretto255_BYTES, 0);
  if (crypto_scalarmult_ristretto255((unsigned char*)v.data(), 
    (unsigned char*)r.data(), (unsigned char*)u.data()) != 0) {
    throw std::domain_error(
      "Could not compute group multiplication in blind_token");
  }
  return v;
}

string blind_token(const string& token, const string& r) {
  return blind_element(value_to_element(token), r);
}

string unblind_element(const string&v, const string& r) {
  if (v.size() != crypto_core_ristretto255_BYTES) {
    throw std::invalid_argument("Group element v incorrect size");
  }
  if (r.size() != crypto_core_ristretto255_SCALARBYTES) {
    throw std::invalid_argument("Scalar r incorrect size");
  }
  unsigned char r_inv[crypto_core_ristretto255_SCALARBYTES];
  crypto_core_ristretto255_scalar_invert(
    r_inv, (unsigned char*)r.data());
  string u(crypto_core_ristretto255_BYTES, 0);
  if (crypto_scalarmult_ristretto255((unsigned char*)u.data(), 
    r_inv, (unsigned char*)v.data()) != 0) {
    throw std::domain_error(
      "Could not compute group multiplication in unblind_element");
  }
  return u;
}