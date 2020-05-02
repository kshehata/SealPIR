#pragma once

#include "pir.hpp"
#include <map>
#include <memory>
#include <vector>
#include "pir_client.hpp"
#include "pir_database.hpp"

using seal::GaloisKeys;

class PIRServer {
  public:
    PIRServer(const seal::EncryptionParameters &params, const PirParams &pir_params);

    // NOTE: server takes over ownership of db and frees it when it exits.
    // Caller cannot free db
    void set_database(std::unique_ptr<std::vector<seal::Plaintext>> &&db);

    std::vector<seal::Ciphertext> expand_query(
            const seal::Ciphertext &encrypted, std::uint32_t m,
            const GaloisKeys& galkey);

    PirReply generate_reply(PirQuery query, const GaloisKeys& galkey);

  private:
    seal::EncryptionParameters params_; // SEAL parameters
    PirParams pir_params_;              // PIR parameters
    std::unique_ptr<Database> db_;
    bool is_db_preprocessed_;
    std::unique_ptr<seal::Evaluator> evaluator_;

    void decompose_to_plaintexts_ptr(const seal::Ciphertext &encrypted, seal::Plaintext *plain_ptr, int logt);
    std::vector<seal::Plaintext> decompose_to_plaintexts(const seal::Ciphertext &encrypted);
    void multiply_power_of_X(const seal::Ciphertext &encrypted, seal::Ciphertext &destination,
                             std::uint32_t index);
};
