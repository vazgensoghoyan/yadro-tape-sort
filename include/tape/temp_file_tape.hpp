#pragma once // tape/temp_file_tape.hpp

#include <memory>
#include <string>

#include "tape/interface_tape.hpp"
#include "tape/file_tape.hpp"

namespace tape_sort::tape {

// Обязан сам создавать файл. Но получает на вход название файла, путь к нему
// Если такой файл уже существует, ошибка
// Деструктор удаляет временный файл

class TempFileTape final : public ITape {
public:
    TempFileTape(const std::string& dir_path, size_t size);
    ~TempFileTape() override;

    int32_t read() override;
    void write(int32_t value) override;

    void move_right() override;
    void move_left() override;

    void rewind() override;

    bool is_bof() const override;
    bool is_eof() const override;

    size_t size() const override;
    size_t position() const override;

    const std::string& file_path() const;

private:
    void zeros_init_file(size_t size);

    static std::string generate_temp_path(const std::string& dir);

private:
    std::string file_path_;
    std::unique_ptr<FileTape> tape_;
};

} // namespace tape_sort::tape
