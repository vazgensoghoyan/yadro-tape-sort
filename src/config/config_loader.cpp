#include "config/config_loader.hpp"

#include "utils/logger.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

using namespace tape_sort::config;

AppConfig ConfigLoader::load(const std::string& path) {
    LOG_INFO("ConfigLoader: loaded AppConfig");
    return {
        TapeConfig {
            .read_delay_ms = 1,
            .write_delay_ms = 1,
            .move_delay_ms = 1,
            .rewind_delay_ms = 3,
        },
        MemoryConfig {
            .limit_bytes = 1024 * 1024
        },
        FileSystemConfig {
            .tmp_dir = "./tmp"
        },
        LogConfig {
            .enabled = true
        }
    };
}
