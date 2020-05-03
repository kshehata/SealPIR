#include "pir_database.hpp"
#include <random>

using namespace std;

void usage() {
    cout << "Usage: gen_db [num items] [bytes per item] [filename]" << endl;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        usage();
        return 1;
    }

    auto number_of_items = atol(argv[1]);
    auto size_per_item = atol(argv[2]);
    string filename(argv[3]);

    if (number_of_items < 1) {
        cerr << "Must generate at least 1 item" << endl;
        return 2;
    }
    if (size_per_item < 1) {
        cerr << "Size per item must be at least 1 byte" << endl;
        return 2;
    }

    auto pbdb = make_unique<PIRDatabase>();
    cout << "Generating database of " << number_of_items << " of " << size_per_item
        << " bytes" << endl;

    auto value(make_unique<uint8_t[]>(size_per_item));
    random_device rd;
    for (uint64_t i = 0; i < number_of_items; i++) {
        for (uint64_t j = 0; j < size_per_item; j++) {
            value.get()[j] = rd() % 256;
        }
        auto item = pbdb->add_item();
        item->set_value(value.get(), size_per_item);
    }

    cout << "Writing output to " << filename << endl;
    save_database_to_file(*pbdb, filename);
    return 0;
}
