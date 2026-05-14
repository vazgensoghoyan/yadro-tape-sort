#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

#include "config/config_loader.hpp"
#include "utils/logger.hpp"

#include "tape/file_tape.hpp"
#include "tape/delay_tape_decorator.hpp"

#include "sorter/tape_sorter.hpp"

using namespace tape_sort::config;
using namespace tape_sort::tape;
using namespace tape_sort::sorter;

enum ExitCode {
    SUCCESS = 0,
    USAGE_ERROR = 1,
    RUNTIME_ERROR = 2,
    UNKNOWN_ERROR = 3
};

int main(int argc, char** argv) {
    try {
        if (argc != 4) {
            std::cout << "Usage: tape_sort <input_file> <output_file> <config_file>\n";
            return USAGE_ERROR;
        }

        std::string input_path  = argv[1];
        std::string output_path = argv[2];
        std::string config_path = argv[3];

        const AppConfig config = ConfigLoader::load(config_path);

        FileTape input_file(input_path);
        FileTape output_file(output_path);

        DelayTapeDecorator input_tape(input_file, config.tape);
        DelayTapeDecorator output_tape(output_file, config.tape);

        TapeSorter sorter(config);

        LOG_INFO("Starting sort: {} -> {}", input_path, output_path);
        
        sorter.sort(input_tape, output_tape);

        return SUCCESS;

    } catch (const std::exception& ex) {
        LOG_ERROR("Fatal error: {}", ex.what());
        return RUNTIME_ERROR;

    } catch (...) {
        LOG_ERROR("Unknown fatal error");
        return UNKNOWN_ERROR;
    }
}
