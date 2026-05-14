#include "tape/file_tape.hpp"

#include <stdexcept>
#include <vector>
#include <filesystem>

using namespace tape_sort::tape;

FileTape::FileTape(const std::string& file_path) : file_path_(file_path) {
    file_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);

    if (!file_.is_open())
        throw std::runtime_error("Failed to open tape file: " + file_path_);
    
    head_pos_ = 0;
    tape_size_ = std::filesystem::file_size(file_path_);
}

void FileTape::ensure_opened() const {
    if (!file_.is_open())
        throw std::runtime_error("Tape file is not open: " + file_path_);
}

void FileTape::validate_position() const {
    if (head_pos_ >= tape_size_)
        throw std::runtime_error("Tape head out of range");
}

void FileTape::apply_position() {
    file_.clear();
    file_.seekp(head_pos_ * sizeof(int32_t), std::ios::beg);
}

int FileTape::read() {
    ensure_opened();
    validate_position();

    file_.clear();
    apply_position();

    int32_t value = 0;
    file_.read(reinterpret_cast<char*>(&value), sizeof(value));

    if (!file_)
        throw std::runtime_error("Failed to read from tape: " + file_path_);

    return value;
}

void FileTape::write(int32_t value) {
    ensure_opened();
    validate_position();

    file_.clear();
    apply_position();

    file_.write(reinterpret_cast<const char*>(&value), sizeof(value));

    if (!file_)
        throw std::runtime_error("Failed to write to tape: " + file_path_);

    file_.flush();

    if (head_pos_ == tape_size_ - 1)
        ++tape_size_;
}

void FileTape::move_right() {
    if (can_move_right())
        ++head_pos_;
}

void FileTape::move_left() {
    if (can_move_left())
        --head_pos_;
}

void FileTape::rewind() {
    head_pos_ = 0;
}

bool FileTape::can_move_right() const {
    return head_pos_ < tape_size_ - 1;
}

bool FileTape::can_move_left() const {
    return head_pos_ > 0;
}
