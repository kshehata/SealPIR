#ifndef SEALPIR_RPC_SERVER_H
#define SEALPIR_RPC_SERVER_H value

#include "pir.hpp"
#include "pir.pb.h"
#include <vector>

using std::string;
using std::unique_ptr;
using std::uint8_t;
using std::uint64_t;
using std::vector;
using seal::EncryptionParameters;
using seal::Plaintext;
using pir::PIRDatabase;

// TODO(kshehata): This would be a lot cleaner if we made the DB a proper class
// and not just a vector.
typedef std::vector<Plaintext> Database;

// Generate and pre-process database based on an array of byte values.
unique_ptr<Database> gen_database(
  const std::uint8_t* const bytes,
  std::uint64_t ele_num, std::uint64_t ele_size,
  const EncryptionParameters& params, const PirParams& pir_params);

// Generate and pre-process database based on a Protocol Buffer.
std::unique_ptr<Database> gen_database(
    const PIRDatabase& pbdb,
    const EncryptionParameters& params, const PirParams& pir_params);

// Load DB from a file in protocol buffer text format.
unique_ptr<PIRDatabase> load_database_from_file(const string& filename);

// Save DB to a file in protocol buffer text format.
void save_database_to_file(const PIRDatabase& db, const string& filename);

// Helper to preprocess database.
void preprocess_database(Database* db, const EncryptionParameters& params);

#endif  // SEALPIR_RPC_SERVER_H
