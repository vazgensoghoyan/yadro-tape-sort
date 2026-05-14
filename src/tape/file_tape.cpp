#include "tape/file_tape.hpp"

using namespace tape_sort::tape;

FileTape::FileTape(const std::string& path) : file_path_(path) {
    file_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);

    if (!file_.is_open())
        throw std::runtime_error("Failed to open tape file: " + file_path_);

    const auto size_bytes = std::filesystem::file_size(file_path_);

    if (size_bytes % CELL_SIZE)
        throw std::runtime_error("Wrong file format: " + file_path_);

    tape_size_ = static_cast<size_t>(size_bytes / CELL_SIZE);
    head_pos_ = 0;
}

size_t FileTape::size() const {
    return tape_size_;
}

size_t FileTape::position() const {
    return head_pos_;
}

const std::string& FileTape::file_path() const {
    return file_path_;
}

std::streamoff FileTape::byte_offset(size_t pos) const {
    return static_cast<std::streamoff>(pos * CELL_SIZE);
}

void FileTape::seek_to_curr_position() {
    const auto offset = byte_offset(head_pos_);

    file_.seekg(offset, std::ios::beg);
    file_.seekp(offset, std::ios::beg);

    if (!file_)
        throw std::runtime_error("Seek failed on tape file: " + file_path_);
}

void FileTape::ensure_opened() const {
    if (!file_.is_open())
        throw std::runtime_error("Tape file is not open: " + file_path_);
}

void FileTape::ensure_position() const {
    if (head_pos_ >= tape_size_)
        throw std::out_of_range("Out of tape bounds");
}

int32_t FileTape::read() {
    ensure_opened();
    ensure_position();

    file_.clear();
    seek_to_curr_position();

    int32_t value = 0;
    file_.read(reinterpret_cast<char*>(&value), sizeof(value));

    if (!file_)
        throw std::runtime_error("Failed to read from tape: " + file_path_);

    return value;
}

void FileTape::write(int32_t value) {
    ensure_opened();
    ensure_position();

    file_.clear();
    seek_to_curr_position();

    file_.write(reinterpret_cast<const char*>(&value), sizeof(value));

    if (!file_)
        throw std::runtime_error("Failed to write to tape: " + file_path_);
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
    file_.clear();
}

bool FileTape::can_move_right() const {
    return head_pos_ + 1 < tape_size_;
}

bool FileTape::can_move_left() const {
    return head_pos_ > 0;
}
