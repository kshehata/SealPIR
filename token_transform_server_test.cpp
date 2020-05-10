#include "token_transform_server.hpp"
#include "token_transforms.hpp"

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sodium.h>
#include <vector>

namespace epione {
namespace testing {

using std::vector;
using std::cout;
using std::endl;

using namespace ::testing;

const unsigned char sample_k_array[] = {
  0x78, 0x34, 0x39, 0xef, 0x7c, 0xfa, 0x86, 0xee,
  0xc5, 0xda, 0x86, 0x72, 0xbe, 0xa6, 0x9c, 0x39,
  0x29, 0x9e, 0x1f, 0x23, 0xd0, 0x8b, 0xeb, 0xca,
  0x5f, 0xcb, 0xde, 0xf9, 0x6c, 0x19, 0x45, 0x07};

const string sample_k((char*)sample_k_array, sizeof(sample_k_array));

const unsigned char sample_token_array[] =
    {0x33, 0x15, 0x16, 0x23, 0x66, 0x49, 0x04, 0x26,
     0xcc, 0x07, 0xf6, 0xcb, 0x26, 0x9c, 0x4a, 0x51,
     0x02, 0x17, 0x44, 0xfa, 0x2a, 0x1f, 0x0a, 0xa8,
     0x87, 0x53, 0x4a, 0x75, 0x61, 0x33, 0x15, 0x17,
     0xfd, 0xd1, 0x26, 0x99, 0xc2, 0x9e, 0xaa, 0x56,
     0xe8, 0xb6, 0x72, 0x8e, 0xf9, 0xc2, 0xa4, 0x88,
     0xaf, 0xe4, 0xdf, 0x94, 0x34, 0x7b, 0xc1, 0x24,
     0x27, 0x9d, 0x2f, 0xbd, 0xf8, 0x64, 0x41, 0xa0,
     0x2e, 0x8e, 0xd5, 0x60, 0x53, 0x1b, 0xcc, 0xe9,
     0xec, 0xca, 0xdc, 0xae, 0xd7, 0x7f, 0x43, 0x81,
     0x65, 0x23, 0xfe, 0x30, 0xb9, 0x12, 0xa2, 0xe1,
     0x46, 0x2f, 0x47, 0x0c, 0xab, 0xb5, 0xca, 0xc2,
     0x33, 0x55, 0xe8, 0x9f, 0x5f, 0x98, 0x71, 0x8a,
     0x09, 0x7c, 0xc8, 0xec, 0x5c, 0x5e, 0xea, 0x84,
     0x7f, 0x19, 0xfc, 0x26, 0x39, 0xa8, 0x94, 0x1f,
     0x5a, 0xbd, 0x6e, 0x1b, 0x67, 0x93, 0xa2, 0xf3,
     0x4d, 0x3d, 0xf2, 0xad, 0xeb, 0xd2, 0x90, 0x0f,
     0xfd, 0xb2, 0x06, 0x3b, 0x41, 0x01, 0x24, 0x29,
     0xd4, 0xab, 0xc3, 0x3a, 0x62, 0x46, 0x4b, 0x39,
     0xfd, 0x08, 0x16, 0x5b, 0x96, 0x35, 0x48, 0x87};

TEST(TokenTransformServerTest, BlindSingleToken) {
  ASSERT_THAT(sodium_init(), Ge(0));
  TokenTransformServer server = TokenTransformServer::Create(sample_k);

  string sample_token = string((char*)sample_token_array, TOKEN_SIZE);
  TokenSet input_ts;
  TokenSet output_ts;
  input_ts.add_token(value_to_element(sample_token));

  EXPECT_THAT(server.Blind(nullptr, &input_ts, &output_ts).ok(), IsTrue());

  string expected_output_token =
      blind_token(sample_token, sample_k);

  EXPECT_THAT(output_ts.token(), ElementsAre(expected_output_token));
}

TEST(TokenTransformServerTest, BlindTokenSet) {
  ASSERT_THAT(sodium_init(), Ge(0));
  TokenTransformServer server = TokenTransformServer::Create(sample_k);

  TokenSet input_ts;
  TokenSet output_ts;
  vector<string> expected_output;

  for (int i = 0; i < sizeof(sample_token_array); i += TOKEN_SIZE) {
    string t((char*)sample_token_array + i, TOKEN_SIZE);
    expected_output.push_back(blind_token(t, sample_k));
    input_ts.add_token(value_to_element(t));
  }

  EXPECT_THAT(server.Blind(nullptr, &input_ts, &output_ts).ok(), IsTrue());

  // Check that the blinded values for all tokens are present
  EXPECT_THAT(output_ts.token(), 
    UnorderedElementsAreArray(expected_output));

  // But should *not* be in the default order
  EXPECT_THAT(output_ts.token(),
    Not(ElementsAreArray(expected_output)));
}


TEST(TokenTransformServerTest, BlindTokenTooShortReturnsInvalidArgumentStatus) {
  ASSERT_THAT(sodium_init(), Ge(0));
  TokenTransformServer server = TokenTransformServer::Create(sample_k);

  TokenSet input_ts;
  TokenSet output_ts;

  for (int i = 0; i < sizeof(sample_token_array); i += TOKEN_SIZE) {
    string t((char*)sample_token_array + i, TOKEN_SIZE);
    auto p = value_to_element(t);
    if (i == 2 * TOKEN_SIZE) {
      p.resize(p.size() - 2);
    }
    input_ts.add_token(p);
  }

  EXPECT_THAT(server.Blind(nullptr, &input_ts, &output_ts).error_code(), 
    Eq(grpc::INVALID_ARGUMENT));
}

TEST(TokenTransformServerTest, CreateWithEmptyKThrowsInvalidArgument) {
  ASSERT_THROW(TokenTransformServer::Create(""), std::invalid_argument);
}

TEST(TokenTransformServerTest, CreateWithShortKThrowsInvalidArgument) {
  auto k = sample_k;
  k.resize(k.size() - 2);
  ASSERT_THROW(TokenTransformServer::Create(k), std::invalid_argument);
}

}  // namespace testing
}  // namespace epione
