#include "token_transforms.hpp"

#include <gtest/gtest.h>
#include <sodium.h>

namespace testing {

void EXPECT_NOT_ALL_ZERO(const string& v) {
  for (auto c : v) {
    if (c != (char)0) {
      return;
    }
  }
  FAIL() << "Value is all zero";
}

void EXPECT_NOT_EQUAL(const string& v1, const string& v2) {
  ASSERT_EQ(v1.size(), v2.size());
  for (size_t i = 0; i < v1.size(); ++i) {
    if (v1[i] != v2[i]) {
      return;
    }
  }
  FAIL() << "Both strings equal";
}

void EXPECT_EQUAL(const string& v1, const string& v2) {
  ASSERT_EQ(v1.size(), v2.size());
  for (size_t i = 0; i < v1.size(); ++i) {
    if (v1[i] != v2[i]) {
      FAIL() << "Both strings not equal";
    }
  }
}

TEST(TokenTransformsTest, GenerateRandomScalar) {
  ASSERT_GE(sodium_init(), 0);
  auto r = generate_random_scalar();
  ASSERT_EQ(r.size(), crypto_core_ristretto255_SCALARBYTES);
  EXPECT_NOT_ALL_ZERO(r);
}

const unsigned char sample_token_1_array[] =
    {0xDE, 0xAD, 0xBE, 0xEF, 0xAA, 0x55, 0x55, 0xAA,
     0x00, 0xFF, 0xF0, 0x0F, 0xA5, 0x5A, 0x99, 0x42};
const unsigned char sample_token_2_array[] =
    {0xDE, 0xAD, 0xBE, 0xEF, 0xAA, 0x55, 0x55, 0xAA,
     0x00, 0xFF, 0xF0, 0x0F, 0xA5, 0x5A, 0x99, 0x41};

const string sample_token_1((char*)sample_token_1_array,
  sizeof(sample_token_1_array));
const string sample_token_2((char*)sample_token_2_array,
  sizeof(sample_token_2_array));

TEST(TokenTransformsTest, ValueToGroup) {
  ASSERT_GE(sodium_init(), 0);  
  ASSERT_EQ(sample_token_1.size(), 16);
  ASSERT_EQ(sample_token_2.size(), 16);

  string group_1 = value_to_group(sample_token_1);
  ASSERT_EQ(group_1.size(), crypto_core_ristretto255_BYTES);
  EXPECT_NOT_ALL_ZERO(group_1);

  string group_2 = value_to_group(sample_token_2);
  ASSERT_EQ(group_2.size(), crypto_core_ristretto255_BYTES);
  EXPECT_NOT_ALL_ZERO(group_2);

  EXPECT_NOT_EQUAL(group_1, group_2);
}

TEST(TokenTransformsTest, BlindToken) {
  auto r = generate_random_scalar();
  auto v = blind_token(sample_token_1, r);
  ASSERT_EQ(v.size(), crypto_core_ristretto255_BYTES);
  EXPECT_NOT_ALL_ZERO(v);
  EXPECT_NOT_EQUAL(v, value_to_group(sample_token_1));
}

TEST(TokenTransformsTest, BlindToken_RTooShortThrows) {
  auto r = generate_random_scalar();
  r.resize(r.size() / 2);
  ASSERT_THROW(blind_token(sample_token_1, r), std::invalid_argument);
}

TEST(TokenTransformsTest, UnblindToken) {
  auto r = generate_random_scalar();
  auto p = value_to_group(sample_token_1);
  auto v = blind_token(sample_token_1, r);
  auto u = unblind_element(v, r);
  ASSERT_EQ(u.size(), crypto_core_ristretto255_BYTES);
  EXPECT_EQUAL(u, p);
}

TEST(TokenTransformsTest, BlindElement_RTooShortThrows) {
  auto r = generate_random_scalar();
  auto p = value_to_group(sample_token_1);
  r.resize(r.size() / 2);
  ASSERT_THROW(unblind_element(p, r), std::invalid_argument);
}

TEST(TokenTransformsTest, BlindElement_ElementTooShortThrows) {
  auto r = generate_random_scalar();
  auto p = value_to_group(sample_token_1);
  p.resize(p.size() / 2);
  ASSERT_THROW(unblind_element(p, r), std::invalid_argument);
}

}  // namespace testing
