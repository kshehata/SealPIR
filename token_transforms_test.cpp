#include "token_transforms.hpp"

#include <gtest/gtest.h>
#include <sodium.h>

namespace testing {

TEST(TokenTransformsTest, GenerateRandomScalar) {
  ASSERT_EQ(sodium_init(), 0);
  auto r = generate_random_scalar();
  ASSERT_EQ(r.size(), crypto_core_ristretto255_SCALARBYTES);
  for (auto c : r) {
    EXPECT_NE(c, (char)0);
  }
}

}  // namespace testing
