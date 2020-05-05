#include <iostream>
#include "../Lab_04/write_data_pnm_v2.h"
#include "conversions_color_spaces.h"

int main(int argc, char *argv[]) {

    if (argc != 4 && argc != 6) {
        std::cerr << "Wrong number of arguments. Syntax:\n<lab5>.exe <file_in> <file_out> <conversion> [<offset> <multiplier>]\n";
        return 1;
    }

    const char *file_in_name = argv[1];
    const char *file_out_name = argv[2];

    unsigned conversion = argv[3][0] - '0';

    int offset;
    long double multiplier;
    if (argc == 6) {
        offset = std::stoi(argv[4]);
        multiplier = std::stold(argv[5]);
    }

    if (conversion > 5 || ((conversion == 0 || conversion == 1) && argc != 6) || (conversion > 1 && argc == 6)) {
        std::cerr << "Wrong arguments\n";
        return 1;
    }

    return 0;
}
