#include "pir_database.hpp"

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

using std::cout;
using std::endl;
using std::move;

std::unique_ptr<Database> gen_database(
  const std::uint8_t* const bytes,
  std::uint64_t ele_num, std::uint64_t ele_size,
  const EncryptionParameters& params, const PirParams& pir_params) {

    uint32_t logt = floor(log2(params.plain_modulus().value()));
    uint32_t N = params.poly_modulus_degree();

    // number of FV plaintexts needed to represent all elements
    uint64_t total = plaintexts_per_db(logt, N, ele_num, ele_size);

    // number of FV plaintexts needed to create the d-dimensional matrix
    uint64_t prod = 1;
    for (uint32_t i = 0; i < pir_params.nvec.size(); i++) {
        prod *= pir_params.nvec[i];
    }
    uint64_t matrix_plaintexts = prod;
    assert(total <= matrix_plaintexts);

    auto result = std::make_unique<Database>();
    result->reserve(matrix_plaintexts);

    uint64_t ele_per_ptxt = elements_per_ptxt(logt, N, ele_size);
    uint64_t bytes_per_ptxt = ele_per_ptxt * ele_size;

    uint64_t db_size = ele_num * ele_size;

    uint64_t coeff_per_ptxt = ele_per_ptxt * coefficients_per_element(logt, ele_size);
    assert(coeff_per_ptxt <= N);

    cout << "Server: total number of FV plaintext = " << total << endl;
    cout << "Server: elements packed into each plaintext " << ele_per_ptxt << endl; 

    uint32_t offset = 0;

    for (uint64_t i = 0; i < total; i++) {

        uint64_t process_bytes = 0;

        if (db_size <= offset) {
            break;
        } else if (db_size < offset + bytes_per_ptxt) {
            process_bytes = db_size - offset;
        } else {
            process_bytes = bytes_per_ptxt;
        }

        // Get the coefficients of the elements that will be packed in plaintext i
        vector<uint64_t> coefficients = bytes_to_coeffs(logt, bytes + offset, process_bytes);
        offset += process_bytes;

        uint64_t used = coefficients.size();

        assert(used <= coeff_per_ptxt);

        // Pad the rest with 1s
        for (uint64_t j = 0; j < (N - used); j++) {
            coefficients.push_back(1);
        }

        Plaintext plain;
        vector_to_plaintext(coefficients, plain);
        // cout << i << "-th encoded plaintext = " << plain.to_string() << endl; 
        result->push_back(move(plain));
    }

    // Add padding to make database a matrix
    uint64_t current_plaintexts = result->size();
    assert(current_plaintexts <= total);

#ifdef DEBUG
    cout << "adding: " << matrix_plaintexts - current_plaintexts
         << " FV plaintexts of padding (equivalent to: "
         << (matrix_plaintexts - current_plaintexts) * elements_per_ptxt(logtp, N, ele_size)
         << " elements)" << endl;
#endif

    vector<uint64_t> padding(N, 1);

    for (uint64_t i = 0; i < (matrix_plaintexts - current_plaintexts); i++) {
        Plaintext plain;
        vector_to_plaintext(padding, plain);
        result->push_back(plain);
    }

    preprocess_database(result.get(), params);
    return move(result);
}

std::unique_ptr<Database> gen_database(
    const PIRDatabase& pbdb,
    const EncryptionParameters& params, const PirParams& pir_params) {

    // N means something different here. Need to figure out another
    // way to do consistency check.
    // if (pbdb.item_size() != pir_params.n) {
    //   cout << "DB size mismatch, pb has " << pbdb.item_size()
    //       << " while PIR params has " << pir_params.n << endl;
    //   throw std::invalid_argument("Database size does not match PIR params");
    // }

    uint32_t min_size = 1;
    uint32_t max_size = 0;
    for (const auto& item : pbdb.item()) {
        auto s = item.value().size();
        if (min_size > max_size) {
            min_size = s;
            max_size = s;
        } else if (s > max_size) {
            max_size = s;
        } else if (s < min_size) {
            min_size = s;
        }
    }

    if (min_size != max_size) {
        throw std::invalid_argument("Database item size inconsistent");
    }

    uint64_t number_of_items = pbdb.item_size();
    uint64_t size_per_item = max_size;
    auto db(std::make_unique<uint8_t[]>(number_of_items * size_per_item));

    for (uint32_t i = 0; i < number_of_items; ++i) {
        std::memcpy(db.get() + (i * size_per_item),
          pbdb.item(i).value().c_str(), size_per_item);
    }

    return move(gen_database(db.get(), number_of_items, size_per_item,
      params, pir_params));
}

unique_ptr<PIRDatabase> load_database_from_file(const string& filename) {
  int fd = open(filename.c_str(), O_RDONLY);
  // unique_ptr<google::protobuf::io::ZeroCopyInputStream> fin
  //     = std::make_unique<google::protobuf::io::FileInputStream>(fd);
  auto db(std::make_unique<PIRDatabase>());
  // if (!google::protobuf::TextFormat::Parse(fin.get(), db.get())) {
  if (!db->ParseFromFileDescriptor(fd)) {
    close(fd);
    throw std::invalid_argument("Could not load db from file");
  }
  close(fd);
  return move(db);
}

void save_database_to_file(const PIRDatabase& db, const string& filename) {
  int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  // unique_ptr<google::protobuf::io::ZeroCopyOutputStream> fout
  //     = std::make_unique<google::protobuf::io::FileOutputStream>(fd);
  // if (!google::protobuf::TextFormat::Print(db, fout.get())) {
  if (!db.SerializeToFileDescriptor(fd)) {
    close(fd);
    throw std::invalid_argument("Could not save db to file");
  }
  close(fd);
}

// Helper to preprocess database.
void preprocess_database(Database* db, const EncryptionParameters& params) {
    cout << "Preprocessing DB" << endl;
    auto context = seal::SEALContext::Create(params, false);
    seal::Evaluator evaluator(context);
    for (uint32_t i = 0; i < db->size(); i++) {
        evaluator.transform_to_ntt_inplace(
            db->operator[](i), params.parms_id());
    }
}

string serialize_ciphertext(const seal::Ciphertext& c) {
    std::ostringstream output;
    c.save(output);
    return output.str();
}

void serialize_ciphertexts(const vector<seal::Ciphertext>& cv,
    pir::Ciphertexts* ctpb) {

    for (const auto& c: cv) {
      ctpb->add_ct(serialize_ciphertext(c));
    }
}

void serialize_pir_query(const PirQuery& pir_query, pir::PIRQuery* query_pb) {
  for (const auto& q : pir_query) {
    serialize_ciphertexts(q, query_pb->add_query());
  }
}


seal::Ciphertext deserialize_ciphertext(const string& s) {
    seal::Ciphertext c;
    std::istringstream input(s);
    c.unsafe_load(input);
    return c;
}

vector<seal::Ciphertext> deserialize_ciphertexts(const pir::Ciphertexts& ctpb) {
    vector<seal::Ciphertext> cv;
    cv.reserve(ctpb.ct_size());
    for (const auto& c : ctpb.ct()) {
        cv.push_back(deserialize_ciphertext(c));
    }
    return cv;
}

PirQuery deserialize_pir_query(const pir::PIRQuery& query_pb) {
  PirQuery pir_query;
  pir_query.reserve(query_pb.query_size());
  for (const auto& q : query_pb.query()) {
    pir_query.push_back(deserialize_ciphertexts(q));
  }
  return pir_query;
}
