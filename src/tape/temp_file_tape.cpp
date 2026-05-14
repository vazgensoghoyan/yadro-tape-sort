#include "tape/temp_file_tape.hpp"

#include "utils/logger.hpp"

#include <fstream>
#include <vector>

using namespace tape_sort::tape;

TempFileTape::TempFileTape(const std::string& path, size_t size) : file_path_(path) {
    create_and_init_file(size);
    tape_ = std::make_unique<FileTape>(file_path_);
}

TempFileTape::~TempFileTape() {
    try {
        std::filesystem::remove(file_path_);
        LOG_INFO("Deleted temp tape: {}", file_path_);
    } catch (...) { }
}

void TempFileTape::create_and_init_file(size_t size) {
    namespace fs = std::filesystem;

    fs::path path(file_path_);
    auto dir = path.parent_path();

    if (!dir.empty() && !fs::exists(dir))
        fs::create_directories(dir);

    if (fs::exists(file_path_))
        throw std::runtime_error("Temp file already exists: " + file_path_);

    std::ofstream file(file_path_, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to create temp file: " + file_path_);

    LOG_INFO("Created temp tape: {}", file_path_);

    int32_t zero = 0;

    for (size_t i = 0; i < size; ++i)
        file.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
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

bool TempFileTape::can_move_right() const {
    return tape_->can_move_right();
}

bool TempFileTape::can_move_left() const {
    return tape_->can_move_left();
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
