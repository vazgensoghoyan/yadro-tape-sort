#pragma once //app/app.hpp

#include <string>
#include "config/config_structs.hpp"

namespace tape_sort::app {

enum class ExitCode : int {
    Success = 0,
    UsageError = 1,
    RuntimeError = 2,
    UnknownError = 3
};

class App {
public:
    ExitCode run(int argc, char** argv);

private:
    struct Args {
        std::string input_path;
        std::string output_path;
        std::string config_path;
    };

    Args parse_args(int argc, char** argv);

    void execute(const Args& args, const config::AppConfig& config);

    static void create_output_tape_file(const std::string& path, size_t size);
};

} // namespace tape_sort::app
