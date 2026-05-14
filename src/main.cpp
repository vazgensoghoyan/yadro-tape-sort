#include <iostream>
#include <string>

#include "config/config_loader.hpp"

using namespace tape_sort;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: tape_sort <input> <output> <config>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    std::string config_file = argv[3];

    auto config = config::ConfigLoader::load(config_file);

    return 0;
}
