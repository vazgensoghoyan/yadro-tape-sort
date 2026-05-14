#pragma once // tape/file_tape.hpp

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "tape/interface_tape.hpp"

namespace tape_sort::tape {

class FileTape final : public ITape {
public:
    explicit FileTape(const std::string& path);

    ~FileTape() override = default;

    int32_t read() override;
    void write(int32_t value) override;

    void move_right() override;
    void move_left() override;
    void rewind() override;

    bool can_move_right() const override;
    bool can_move_left() const override;

    size_t size() const override;
    size_t position() const override;

    const std::string& file_path() const;

private:
    void ensure_opened() const;
    void ensure_position() const;
    void seek_to_curr_position();
    std::streamoff byte_offset(size_t pos) const;

private:
    static constexpr size_t CELL_SIZE = sizeof(int32_t);

    std::fstream file_;
    std::string file_path_;

    size_t head_pos_;
    size_t tape_size_;
};

} // namespace tape_sort::tape
