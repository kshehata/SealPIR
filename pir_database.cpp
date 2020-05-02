#include "pir_database.hpp"

#include <iostream>

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
