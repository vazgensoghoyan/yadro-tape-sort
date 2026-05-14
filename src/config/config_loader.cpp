#include "config/config_loader.hpp"

#include "utils/logger.hpp"

#include <fstream>
#include <stdexcept>
#include <unordered_map>

using namespace tape_sort::config;

namespace { // helper functions

std::string trim(const std::string& s);
std::string remove_comment(const std::string& line);
int parse_int(const std::string& value);
size_t parse_size_t(const std::string& value);

} // namespace

AppConfig ConfigLoader::load(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open())
        throw std::runtime_error("Failed to open config file: " + path);

    std::unordered_map<std::string, std::string> values;

    std::string line;
    size_t line_number = 0;

    while (std::getline(file, line)) {
        ++line_number;

        line = remove_comment(line);
        line = trim(line);

        if (line.empty())
            continue;

        const size_t eq_pos = line.find('=');

        if (eq_pos == std::string::npos)
            throw std::runtime_error("Invalid config line " + std::to_string(line_number) + ": '=' not found");

        std::string key = trim(line.substr(0, eq_pos));
        std::string value = trim(line.substr(eq_pos + 1));

        if (key.empty())
            throw std::runtime_error("Invalid config line " + std::to_string(line_number) + ": empty key");

        values[key] = value;
    }

    AppConfig config{};

    auto require = [&](const std::string& key) -> const std::string& {
        auto it = values.find(key);

        if (it == values.end())
            throw std::runtime_error("Missing config key: " + key);

        return it->second;
    };

    config.tape.read_delay_ms = parse_int(require("tape.read_delay_ms"));
    config.tape.write_delay_ms = parse_int(require("tape.write_delay_ms"));
    config.tape.move_delay_ms = parse_int(require("tape.move_delay_ms"));
    config.tape.rewind_delay_ms = parse_int(require("tape.rewind_delay_ms"));

    config.memory.limit_bytes = parse_size_t(require("memory.limit_bytes"));

    config.filesystem.tmp_dir = require("filesystem.tmp_dir");

    LOG_INFO("Loaded config from {}", path);

    return config;
}

namespace { // helper functions realizations

bool is_space(char c) {
    return c == ' '  || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

std::string trim(const std::string& s) {
    size_t begin = 0;
    size_t end = s.size();

    while (begin < s.size() && is_space(s[begin])) ++begin;
    while (end > begin && is_space(s[end - 1])) --end;

    return s.substr(begin, end - begin);
}

std::string remove_comment(const std::string& line) {
    const size_t pos = line.find("//");

    if (pos == std::string::npos)
        return line;

    return line.substr(0, pos);
}

int parse_int(const std::string& value) {
    try {
        size_t pos = 0;

        int result = std::stoi(value, &pos);

        if (pos != value.size())
            throw std::runtime_error("");

        return result;

    } catch (const std::exception&) {
        throw std::runtime_error("Invalid int value: " + value);
    }
}

size_t parse_size_t(const std::string& value) {
    try {
        size_t pos = 0;

        size_t result = std::stoull(value, &pos);

        if (pos != value.size())
            throw std::runtime_error("");

        return result;

    } catch (const std::exception&) {
        throw std::runtime_error("Invalid size_t value: " + value);
    }
}

} // namespace
