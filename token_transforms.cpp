#include "token_transforms.hpp"

#include <sodium.h>

// TODO(kshehata): Need to call sodium_init() somewhere

string generate_random_scalar() {
  string r(crypto_core_ristretto255_SCALARBYTES, 0);
  crypto_core_ristretto255_scalar_random((unsigned char*)r.data());
  return r;
}