#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdint>

#include "tape/file_tape.hpp"

using namespace tape_sort::tape;

namespace fs = std::filesystem;

class FileTapeTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "tape_sort_tests";
        fs::create_directories(test_dir_);

        tape_path_ = test_dir_ / "test_tape.bin";
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(test_dir_, ec);
    }

    void create_tape_file(const std::vector<int32_t>& values) {
        std::ofstream out(tape_path_, std::ios::binary);

        if (!out.is_open())
            throw std::runtime_error("Failed to create test tape");

        for (int32_t value : values) {
            out.write(
                reinterpret_cast<const char*>(&value),
                sizeof(value)
            );
        }
    }

protected:
    fs::path test_dir_;
    fs::path tape_path_;
};

TEST_F(FileTapeTest, OpensValidTapeFile)
{
    create_tape_file({1, 2, 3, 4});

    FileTape tape(tape_path_.string());

    EXPECT_EQ(tape.size(), 4);
    EXPECT_EQ(tape.position(), 1);

    EXPECT_FALSE(tape.is_bof());
    EXPECT_FALSE(tape.is_eof());
}

TEST_F(FileTapeTest, ThrowsIfFileDoesNotExist)
{
    EXPECT_THROW(
        FileTape("missing_file.bin"),
        std::runtime_error
    );
}

TEST_F(FileTapeTest, ThrowsIfFileSizeIsInvalid)
{
    std::ofstream out(tape_path_, std::ios::binary);

    char garbage[3] = {1, 2, 3};
    out.write(garbage, sizeof(garbage));

    out.close();

    EXPECT_THROW(
        FileTape(tape_path_.string()),
        std::runtime_error
    );
}

TEST_F(FileTapeTest, ReadsCurrentElement)
{
    create_tape_file({10, 20, 30});

    FileTape tape(tape_path_.string());

    EXPECT_EQ(tape.read(), 10);

    tape.move_right();
    EXPECT_EQ(tape.read(), 20);

    tape.move_right();
    EXPECT_EQ(tape.read(), 30);
}

TEST_F(FileTapeTest, WriteOverwritesCurrentCell)
{
    create_tape_file({1, 2, 3});

    {
        FileTape tape(tape_path_.string());

        tape.move_right();
        tape.write(999);
    }

    {
        FileTape tape(tape_path_.string());

        EXPECT_EQ(tape.read(), 1);

        tape.move_right();
        EXPECT_EQ(tape.read(), 999);

        tape.move_right();
        EXPECT_EQ(tape.read(), 3);
    }
}

TEST_F(FileTapeTest, MoveRightChangesPosition)
{
    create_tape_file({1, 2, 3});

    FileTape tape(tape_path_.string());

    EXPECT_EQ(tape.position(), 1);

    tape.move_right();
    EXPECT_EQ(tape.position(), 2);

    tape.move_right();
    EXPECT_EQ(tape.position(), 3);

    tape.move_right();
    EXPECT_TRUE(tape.is_eof());
    EXPECT_EQ(tape.position(), 4);
}

TEST_F(FileTapeTest, MoveLeftChangesPosition)
{
    create_tape_file({1, 2, 3});

    FileTape tape(tape_path_.string());

    tape.move_right();
    tape.move_right();

    EXPECT_EQ(tape.position(), 3);

    tape.move_left();
    EXPECT_EQ(tape.position(), 2);

    tape.move_left();
    EXPECT_EQ(tape.position(), 1);

    tape.move_left();

    EXPECT_TRUE(tape.is_bof());
    EXPECT_EQ(tape.position(), 0);
}

TEST_F(FileTapeTest, RewindMovesHeadToFirstElement)
{
    create_tape_file({1, 2, 3, 4});

    FileTape tape(tape_path_.string());

    tape.move_right();
    tape.move_right();

    EXPECT_EQ(tape.position(), 3);

    tape.rewind();

    EXPECT_EQ(tape.position(), 1);
    EXPECT_EQ(tape.read(), 1);
}

TEST_F(FileTapeTest, ReadThrowsAtBOF)
{
    create_tape_file({1, 2, 3});

    FileTape tape(tape_path_.string());

    tape.move_left();

    EXPECT_TRUE(tape.is_bof());

    EXPECT_THROW(
        tape.read(),
        std::out_of_range
    );
}

TEST_F(FileTapeTest, ReadThrowsAtEOF)
{
    create_tape_file({1, 2, 3});

    FileTape tape(tape_path_.string());

    tape.move_right();
    tape.move_right();
    tape.move_right();

    EXPECT_TRUE(tape.is_eof());

    EXPECT_THROW(
        tape.read(),
        std::out_of_range
    );
}

TEST_F(FileTapeTest, WriteThrowsAtBOF)
{
    create_tape_file({1, 2, 3});

    FileTape tape(tape_path_.string());

    tape.move_left();

    EXPECT_THROW(
        tape.write(123),
        std::out_of_range
    );
}

TEST_F(FileTapeTest, WriteThrowsAtEOF)
{
    create_tape_file({1, 2, 3});

    FileTape tape(tape_path_.string());

    tape.move_right();
    tape.move_right();
    tape.move_right();

    EXPECT_THROW(
        tape.write(123),
        std::out_of_range
    );
}

TEST_F(FileTapeTest, HandlesNegativeValues)
{
    create_tape_file({-10, -20, 30});

    FileTape tape(tape_path_.string());

    EXPECT_EQ(tape.read(), -10);

    tape.move_right();
    EXPECT_EQ(tape.read(), -20);

    tape.move_right();
    EXPECT_EQ(tape.read(), 30);
}

TEST_F(FileTapeTest, HandlesSingleElementTape)
{
    create_tape_file({42});

    FileTape tape(tape_path_.string());

    EXPECT_EQ(tape.size(), 1);
    EXPECT_EQ(tape.read(), 42);

    tape.move_right();

    EXPECT_TRUE(tape.is_eof());
}

TEST_F(FileTapeTest, HandlesEmptyTape)
{
    create_tape_file({});

    FileTape tape(tape_path_.string());

    EXPECT_EQ(tape.size(), 0);

    EXPECT_FALSE(tape.is_bof());
    EXPECT_TRUE(tape.is_eof());

    EXPECT_THROW(
        tape.read(),
        std::out_of_range
    );
}
