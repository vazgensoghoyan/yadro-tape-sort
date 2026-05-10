#pragma once // config/config_structs.hpp

#include <cstddef>
#include <string>

namespace tape_sort::config {

struct TapeConfig {
    int read_delay_ms;
    int write_delay_ms;
    int move_delay_ms;
    int rewind_delay_ms;
};

struct MemoryConfig {
    size_t limit_bytes;
};

struct FileSystemConfig {
    std::string tmp_dir;
};

struct LogConfig {
    bool enabled;
};

struct AppConfig {
    TapeConfig tape;
    MemoryConfig memory;
    FileSystemConfig filesystem;
    LogConfig log;
};

} // namespace tape_sort::config
