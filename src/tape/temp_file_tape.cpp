#include "tape/temp_file_tape.hpp"

#include "utils/logger.hpp"

#include <fstream>
#include <vector>
#include <filesystem>

using namespace tape_sort::tape;

TempFileTape::TempFileTape(const std::string& dir, size_t size) {
    file_path_ = generate_temp_path(dir);
    zeros_init_file(size);
    tape_ = std::make_unique<FileTape>(file_path_);
}

TempFileTape::~TempFileTape() {
    try {
        std::filesystem::remove(file_path_);
        LOG_DEBUG("Deleted temp tape: {}", file_path_);
    } catch (...) {
        LOG_ERROR("Failed to delete temp file: {}", file_path_);
    }
}

void TempFileTape::zeros_init_file(size_t size) {
    std::ofstream file(file_path_, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open temp file: " + file_path_);

    int32_t zero = 0;
    for (size_t i = 0; i < size; ++i) {
        file.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
        if (!file)
            throw std::runtime_error("Failed while initializing temp tape: " + file_path_);
    }

    LOG_DEBUG("Created temp tape {} with size {}", file_path_, size);
}

int32_t TempFileTape::read() {
    return tape_->read();
}

void TempFileTape::write(int32_t value) {
    tape_->write(value);
}

void TempFileTape::move_right() {
    tape_->move_right();
}

void TempFileTape::move_left() {
    tape_->move_left();
}

void TempFileTape::rewind() {
    tape_->rewind();
}

bool TempFileTape::is_bof() const {
    return tape_->is_bof();
}

bool TempFileTape::is_eof() const {
    return tape_->is_eof();
}

size_t TempFileTape::size() const {
    return tape_->size();
}

size_t TempFileTape::position() const {
    return tape_->position();
}

const std::string& TempFileTape::file_path() const {
    return file_path_;
}

std::string TempFileTape::generate_temp_path(const std::string& dir) {
    std::filesystem::create_directories(dir);

    std::string tmpl = dir + "/tape_XXXXXX";

    std::vector<char> buffer(tmpl.begin(), tmpl.end());
    buffer.push_back('\0');

    int fd = mkstemp(buffer.data());
    if (fd == -1)
        throw std::runtime_error("mkstemp failed");
    close(fd);

    return std::string(buffer.data());
}