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

inline void print_config(const AppConfig& config) {
    std::cout << "=== AppConfig ===\n";

    std::cout << "[Tape]\n";
    std::cout << "  read_delay_ms: " << config.tape.read_delay_ms << "\n";
    std::cout << "  write_delay_ms: " << config.tape.write_delay_ms << "\n";
    std::cout << "  move_delay_ms: " << config.tape.move_delay_ms << "\n";
    std::cout << "  rewind_delay_ms: " << config.tape.rewind_delay_ms << "\n";

    std::cout << "[Memory]\n";
    std::cout << "  limit_bytes: " << config.memory.limit_bytes << "\n";

    std::cout << "[Filesystem]\n";
    std::cout << "  tmp_dir: " << config.filesystem.tmp_dir << "\n";

    std::cout << "[Log]\n";
    std::cout << "  enabled: " << (config.log.enabled ? "true" : "false") << "\n";

    std::cout << "=================\n";
}

int main(int argc, char** argv) {
    try {
        if (argc != 4) {
            std::cout << "Usage: tape_sort <input_file> <output_file> <config_file>\n";
            return 1;
        }

        std::string input_path  = argv[1];
        std::string output_path = argv[2];
        std::string config_path = argv[3];

        const AppConfig config = ConfigLoader::load(config_path);

        print_config(config);

        FileTape input_file(input_path);
        FileTape output_file(output_path);

        DelayTapeDecorator input_tape(input_file, config.tape);
        DelayTapeDecorator output_tape(output_file, config.tape);

        TapeSorter sorter(config);

        LOG_INFO("Starting sort: {} -> {}", input_path, output_path);
        
        sorter.sort(input_tape, output_tape);

        return 0;

    } catch (const std::exception& ex) {
        LOG_ERROR("Fatal error: {}", ex.what());
        std::cerr << "Error: " << ex.what() << std::endl;
        return 2;

    } catch (...) {
        LOG_ERROR("Unknown fatal error");
        std::cerr << "Unknown error\n";
        return 3;
    }
}
