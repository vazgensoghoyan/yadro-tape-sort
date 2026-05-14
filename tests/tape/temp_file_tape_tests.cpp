// tests/temp_file_tape_tests.cpp

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "tape/temp_file_tape.hpp"

using namespace tape_sort::tape;

namespace fs = std::filesystem;

class TempFileTapeTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "tape_sort_temp_tests";
        fs::create_directories(test_dir_);
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(test_dir_, ec);
    }

    std::vector<fs::path> list_files() const {
        std::vector<fs::path> files;

        if (!fs::exists(test_dir_))
            return files;

        for (auto& p : fs::directory_iterator(test_dir_)) {
            files.push_back(p.path());
        }

        return files;
    }

    fs::path single_file() const {
        auto files = list_files();
        EXPECT_EQ(files.size(), 1);
        return files.empty() ? fs::path{} : files.front();
    }

    std::vector<int32_t> read_all_values(const fs::path& path) {
        std::ifstream in(path, std::ios::binary);

        if (!in.is_open())
            throw std::runtime_error("Failed to open tape file");

        std::vector<int32_t> result;
        int32_t value = 0;

        while (in.read(reinterpret_cast<char*>(&value), sizeof(value))) {
            result.push_back(value);
        }

        return result;
    }

protected:
    fs::path test_dir_;
};

TEST_F(TempFileTapeTest, CreatesTapeFileOnConstruction)
{
    EXPECT_TRUE(list_files().empty());

    {
        TempFileTape tape(test_dir_.string(), 5);
        EXPECT_EQ(list_files().size(), 1);
    }

    EXPECT_TRUE(list_files().empty());
}

TEST_F(TempFileTapeTest, InitializesTapeWithZeros)
{
    TempFileTape tape(test_dir_.string(), 4);

    auto file = single_file();
    auto values = read_all_values(file);

    ASSERT_EQ(values.size(), 4);

    EXPECT_EQ(values[0], 0);
    EXPECT_EQ(values[1], 0);
    EXPECT_EQ(values[2], 0);
    EXPECT_EQ(values[3], 0);
}

TEST_F(TempFileTapeTest, CorrectlyReportsSize)
{
    TempFileTape tape(test_dir_.string(), 7);

    EXPECT_EQ(tape.size(), 7);
}

TEST_F(TempFileTapeTest, InitialPositionIsFirstElement)
{
    TempFileTape tape(test_dir_.string(), 3);

    EXPECT_EQ(tape.position(), 1);
    EXPECT_FALSE(tape.is_bof());
    EXPECT_FALSE(tape.is_eof());
}

TEST_F(TempFileTapeTest, ReadReturnsInitializedValue)
{
    TempFileTape tape(test_dir_.string(), 2);

    EXPECT_EQ(tape.read(), 0);
}

TEST_F(TempFileTapeTest, WriteStoresValue)
{
    TempFileTape tape(test_dir_.string(), 3);

    tape.write(777);
    tape.rewind();

    EXPECT_EQ(tape.read(), 777);
}

TEST_F(TempFileTapeTest, MultipleWritesWorkCorrectly)
{
    TempFileTape tape(test_dir_.string(), 3);

    tape.write(10);

    tape.move_right();
    tape.write(20);

    tape.move_right();
    tape.write(30);

    tape.rewind();

    EXPECT_EQ(tape.read(), 10);

    tape.move_right();
    EXPECT_EQ(tape.read(), 20);

    tape.move_right();
    EXPECT_EQ(tape.read(), 30);
}

TEST_F(TempFileTapeTest, MoveRightChangesPosition)
{
    TempFileTape tape(test_dir_.string(), 3);

    EXPECT_EQ(tape.position(), 1);

    tape.move_right();
    EXPECT_EQ(tape.position(), 2);

    tape.move_right();
    EXPECT_EQ(tape.position(), 3);

    tape.move_right();

    EXPECT_TRUE(tape.is_eof());
    EXPECT_EQ(tape.position(), 4);
}

TEST_F(TempFileTapeTest, MoveLeftChangesPosition)
{
    TempFileTape tape(test_dir_.string(), 3);

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

TEST_F(TempFileTapeTest, RewindMovesHeadToFirstElement)
{
    TempFileTape tape(test_dir_.string(), 5);

    tape.move_right();
    tape.move_right();

    tape.rewind();

    EXPECT_EQ(tape.position(), 1);
}

TEST_F(TempFileTapeTest, ReadThrowsAtBOF)
{
    TempFileTape tape(test_dir_.string(), 3);

    tape.move_left();

    EXPECT_THROW(tape.read(), std::out_of_range);
}

TEST_F(TempFileTapeTest, ReadThrowsAtEOF)
{
    TempFileTape tape(test_dir_.string(), 3);

    tape.move_right();
    tape.move_right();
    tape.move_right();

    EXPECT_THROW(tape.read(), std::out_of_range);
}

TEST_F(TempFileTapeTest, WriteThrowsAtBOF)
{
    TempFileTape tape(test_dir_.string(), 3);

    tape.move_left();

    EXPECT_THROW(tape.write(123), std::out_of_range);
}

TEST_F(TempFileTapeTest, WriteThrowsAtEOF)
{
    TempFileTape tape(test_dir_.string(), 3);

    tape.move_right();
    tape.move_right();
    tape.move_right();

    EXPECT_THROW(tape.write(123), std::out_of_range);
}

TEST_F(TempFileTapeTest, RemovesFileOnDestruction)
{
    std::vector<fs::path> file_before;

    {
        TempFileTape tape(test_dir_.string(), 3);
        file_before = list_files();
        EXPECT_EQ(file_before.size(), 1);
    }

    EXPECT_TRUE(list_files().empty());
}

TEST_F(TempFileTapeTest, CreatesParentDirectories)
{
    fs::path nested = test_dir_ / "a" / "b" / "c";

    TempFileTape tape(nested.string(), 2);

    EXPECT_EQ(list_files().size(), 1);
}

TEST_F(TempFileTapeTest, WritesValuesCorrectly)
{
    TempFileTape tape(test_dir_.string(), 3);

    tape.write(11);
    tape.move_right();
    tape.write(22);
    tape.move_right();
    tape.write(33);

    tape.rewind();

    EXPECT_EQ(tape.read(), 11);
    tape.move_right();
    EXPECT_EQ(tape.read(), 22);
    tape.move_right();
    EXPECT_EQ(tape.read(), 33);
}

TEST_F(TempFileTapeTest, HandlesSingleElementTape)
{
    TempFileTape tape(test_dir_.string(), 1);

    EXPECT_EQ(tape.size(), 1);

    tape.write(999);
    tape.rewind();

    EXPECT_EQ(tape.read(), 999);

    tape.move_right();
    EXPECT_TRUE(tape.is_eof());
}

TEST_F(TempFileTapeTest, HandlesEmptyTape)
{
    TempFileTape tape(test_dir_.string(), 0);

    EXPECT_EQ(tape.size(), 0);
    EXPECT_TRUE(tape.is_eof());

    EXPECT_THROW(tape.read(), std::out_of_range);
}
