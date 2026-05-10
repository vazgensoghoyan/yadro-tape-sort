#pragma once // config/config_loader.hpp

#include <string>

#include "config/config_structs.hpp"

namespace tape_sort::config {

class ConfigLoader {
public:
    static AppConfig load(const std::string& path);
};

} // namespace tape_sort::config
