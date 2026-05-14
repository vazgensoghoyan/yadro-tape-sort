#include "app/app.hpp"

#include <iostream>
#include <stdexcept>

#include "config/config_loader.hpp"
#include "utils/logger.hpp"

#include "tape/file_tape.hpp"
#include "tape/delay_tape_decorator.hpp"

#include "sorter/tape_sorter.hpp"

using namespace tape_sort::app;
using namespace tape_sort::config;
using namespace tape_sort::tape;
using namespace tape_sort::sorter;

ExitCode App::run(int argc, char** argv) {
    try {
        const auto args = parse_args(argc, argv);

        const AppConfig config = ConfigLoader::load(args.config_path);

        execute(args, config);

        return ExitCode::Success;

    } catch (const std::exception& ex) {
        LOG_ERROR("Fatal error: {}", ex.what());
        return ExitCode::RuntimeError;

    } catch (...) {
        LOG_ERROR("Unknown fatal error");
        return ExitCode::UnknownError;
    }
}

App::Args App::parse_args(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: tape_sort <input_file> <output_file> <config_file>\n";
        throw std::invalid_argument("Invalid arguments");
    }

    return Args{
        .input_path = argv[1],
        .output_path = argv[2],
        .config_path = argv[3]
    };
}

void App::execute(const Args& args, const AppConfig& config) {

    FileTape input_file(args.input_path);

    size_t output_size = input_file.size();
    create_output_tape_file(args.output_path, output_size);
    
    FileTape output_file(args.output_path);

    DelayTapeDecorator input_tape(input_file, config.tape);
    DelayTapeDecorator output_tape(output_file, config.tape);

    TapeSorter sorter(config);

    LOG_INFO("Starting sort: {} -> {}", args.input_path, args.output_path);

    sorter.sort(input_tape, output_tape);
}

void App::create_output_tape_file(const std::string& path, size_t size) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
        throw std::runtime_error("Failed to create output tape file: " + path);

    int32_t zero = 0;
    for (size_t i = 0; i < size; ++i) {
        file.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
        if (!file)
            throw std::runtime_error("Failed while initializing output tape: " + path);
    }
}
