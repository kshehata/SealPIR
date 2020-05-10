#include "token_transforms.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sodium.h>

namespace testing {

TEST(TokenTransformsTest, GenerateRandomScalar) {
  ASSERT_THAT(sodium_init(), Ge(0));
  auto r = generate_random_scalar();
  ASSERT_THAT(r, SizeIs(SCALAR_SIZE));
  EXPECT_THAT(r, Not(Each((char)0)));
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
  ASSERT_THAT(sodium_init(), Ge(0));
  ASSERT_THAT(sample_token_1, SizeIs(TOKEN_SIZE));
  ASSERT_THAT(sample_token_2, SizeIs(TOKEN_SIZE));

  string group_1 = value_to_element(sample_token_1);
  ASSERT_THAT(group_1, SizeIs(GROUP_ELEMENT_SIZE));
  EXPECT_THAT(group_1, Not(Each((char)0)));

  string group_2 = value_to_element(sample_token_2);
  ASSERT_THAT(group_2, SizeIs(GROUP_ELEMENT_SIZE));
  EXPECT_THAT(group_2, Not(Each((char)0)));

  EXPECT_THAT(group_1, Not(ElementsAreArray(group_2)));
}


TEST(TokenTransformsTest, IsValidElementTrueForValidElement) {
  string p = value_to_element(sample_token_1);
  ASSERT_THAT(is_valid_element(p), IsTrue());
}

TEST(TokenTransformsTest, IsValidElementFalseOffByOne) {
  string p = value_to_element(sample_token_1);
  p[0] += 1;
  ASSERT_THAT(is_valid_element(p), IsFalse());
}

TEST(TokenTransformsTest, IsValidElementFalseTooShort) {
  string p = value_to_element(sample_token_1);
  p.resize(p.size() - 1);
  ASSERT_THAT(is_valid_element(p), IsFalse());
}

TEST(TokenTransformsTest, BlindElement) {
  auto r = generate_random_scalar();
  auto p = value_to_element(sample_token_1);
  auto v = blind_element(p, r);
  ASSERT_THAT(v, SizeIs(GROUP_ELEMENT_SIZE));
  EXPECT_THAT(v, Not(Each((char)0)));
  EXPECT_THAT(v, Not(ElementsAreArray(p)));
}

TEST(TokenTransformsTest, BlindToken) {
  auto r = generate_random_scalar();
  auto v = blind_token(sample_token_1, r);
  ASSERT_THAT(v, SizeIs(GROUP_ELEMENT_SIZE));
  EXPECT_THAT(v, Not(Each((char)0)));
  EXPECT_THAT(v, Not(ElementsAreArray(sample_token_1)));
}

TEST(TokenTransformsTest, BlindElement_RTooShortThrows) {
  auto r = generate_random_scalar();
  auto p = value_to_element(sample_token_1);
  r.resize(r.size() / 2);
  ASSERT_THROW(blind_element(p, r), std::invalid_argument);
}

TEST(TokenTransformsTest, BlindElement_ElementTooShortThrows) {
  auto r = generate_random_scalar();
  auto p = value_to_element(sample_token_1);
  p.resize(p.size() / 2);
  ASSERT_THROW(blind_element(p, r), std::invalid_argument);
}

TEST(TokenTransformsTest, BlindToken_RTooShortThrows) {
  auto r = generate_random_scalar();
  r.resize(r.size() / 2);
  ASSERT_THROW(blind_token(sample_token_1, r), std::invalid_argument);
}

TEST(TokenTransformsTest, UnblindToken) {
  auto r = generate_random_scalar();
  auto p = value_to_element(sample_token_1);
  auto v = blind_token(sample_token_1, r);
  auto u = unblind_element(v, r);
  ASSERT_THAT(u, SizeIs(GROUP_ELEMENT_SIZE));
  EXPECT_THAT(u, ElementsAreArray(p));
}

TEST(TokenTransformsTest, UnblindElement_RTooShortThrows) {
  auto r = generate_random_scalar();
  auto p = value_to_element(sample_token_1);
  r.resize(r.size() / 2);
  ASSERT_THROW(unblind_element(p, r), std::invalid_argument);
}

TEST(TokenTransformsTest, UnblindElement_ElementTooShortThrows) {
  auto r = generate_random_scalar();
  auto p = value_to_element(sample_token_1);
  p.resize(p.size() / 2);
  ASSERT_THROW(unblind_element(p, r), std::invalid_argument);
}

}  // namespace testing
