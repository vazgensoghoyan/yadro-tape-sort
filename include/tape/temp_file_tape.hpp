#pragma once // tape/temp_file_tape.hpp

#include <memory>
#include <string>

#include "tape/interface_tape.hpp"
#include "tape/file_tape.hpp"

namespace tape_sort::tape {

class TempFileTape final : public ITape {
public:
    TempFileTape(const std::string& path, size_t size);
    ~TempFileTape() override;

    int32_t read() override;
    void write(int32_t value) override;

    void move_right() override;
    void move_left() override;

    void rewind() override;

    bool is_bof() const override;
    bool is_eof() const override;

    size_t size() const;
    size_t position() const;

    const std::string& file_path() const;

private:
    void create_and_init_file(size_t size);

private:
    std::string file_path_;
    std::unique_ptr<FileTape> tape_;
};

} // namespace tape_sort::tape
