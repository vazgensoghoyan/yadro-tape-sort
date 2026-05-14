#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "tape/temp_file_tape.hpp"

using namespace tape_sort::tape;

namespace { // helper

inline std::filesystem::path make_temp_path(const std::string& name) {
    return std::filesystem::temp_directory_path() / name;
}

int32_t read_raw_int(const std::filesystem::path& path, size_t index) {
    std::ifstream file(path, std::ios::binary);
    file.seekg(static_cast<std::streamoff>(index * sizeof(int32_t)));

    int32_t value = 0;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));

    return value;
}

} // namespace

TEST(TempFileTapeTest, CreatesFile) {
    auto path = make_temp_path("temp_tape_create.bin");

    {
        TempFileTape tape(path, 10);
        EXPECT_TRUE(std::filesystem::exists(path));
    }

    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST(TempFileTapeTest, InitializesWithZeros) {
    auto path = make_temp_path("temp_tape_zeros.bin");

    {
        TempFileTape tape(path, 5);

        EXPECT_EQ(read_raw_int(path, 0), 0);
        EXPECT_EQ(read_raw_int(path, 1), 0);
        EXPECT_EQ(read_raw_int(path, 2), 0);
        EXPECT_EQ(read_raw_int(path, 3), 0);
        EXPECT_EQ(read_raw_int(path, 4), 0);
    }

    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST(TempFileTapeTest, WriteReadWorks) {
    auto path = make_temp_path("temp_tape_rw.bin");

    TempFileTape tape(path, 5);

    tape.write(42);
    EXPECT_EQ(tape.read(), 42);

    tape.move_right();
    tape.write(100);
    EXPECT_EQ(tape.read(), 100);

    tape.rewind();
    EXPECT_EQ(tape.read(), 42);
}

TEST(TempFileTapeTest, MovementBounds) {
    auto path = make_temp_path("temp_tape_bounds.bin");

    TempFileTape tape(path, 3);

    EXPECT_TRUE(tape.can_move_right());
    tape.move_right();

    EXPECT_TRUE(tape.can_move_right());
    tape.move_right();

    EXPECT_FALSE(tape.can_move_right());

    EXPECT_TRUE(tape.can_move_left());
    tape.move_left();

    EXPECT_TRUE(tape.can_move_left());
    tape.move_left();

    EXPECT_FALSE(tape.can_move_left());
}

TEST(TempFileTapeTest, SizeAndPosition) {
    auto path = make_temp_path("temp_tape_pos.bin");

    TempFileTape tape(path, 7);

    EXPECT_EQ(tape.size(), 7);
    EXPECT_EQ(tape.position(), 0);

    tape.move_right();
    tape.move_right();

    EXPECT_EQ(tape.position(), 2);
}

TEST(TempFileTapeTest, OverwriteValue) {
    auto path = make_temp_path("temp_tape_overwrite.bin");

    TempFileTape tape(path, 3);

    tape.write(10);
    EXPECT_EQ(tape.read(), 10);

    tape.write(99);
    EXPECT_EQ(tape.read(), 99);
}

TEST(TempFileTapeTest, RAIIDeletesFileEvenOnException) {
    auto path = make_temp_path("temp_tape_raii.bin");

    try {
        TempFileTape tape(path, 5);
        throw std::runtime_error("fail");
    } catch (...) {
    }

    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST(TempFileTapeTest, MultipleInstancesDontConflict) {
    auto path1 = make_temp_path("temp_tape_1.bin");
    auto path2 = make_temp_path("temp_tape_2.bin");

    {
        TempFileTape t1(path1, 3);
        TempFileTape t2(path2, 3);

        t1.write(1);
        t2.write(2);
    }

    EXPECT_FALSE(std::filesystem::exists(path1));
    EXPECT_FALSE(std::filesystem::exists(path2));
}
