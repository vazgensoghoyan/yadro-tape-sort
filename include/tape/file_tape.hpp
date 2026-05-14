#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

#include "tape/interface_tape.hpp"
#include "config/config_structs.hpp"

namespace tape_sort::tape {

class FileTape final : public ITape {
public:
    FileTape(const std::string& file_path);

    ~FileTape() override = default;

    int32_t read() override;
    void write(int32_t value) override;

    void move_right() override;
    void move_left() override;

    void rewind() override;

    bool can_move_right() const override;
    bool can_move_left() const override;

private:
    std::fstream file_;
    std::string file_path_;

    size_t head_pos_;
    size_t tape_size_;
};

} // namespace tape_sort::tape
