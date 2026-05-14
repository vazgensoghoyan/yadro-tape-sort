#include <gtest/gtest.h>
#include <fstream>
#include <vector>
#include <filesystem>

#include "tape/file_tape.hpp"

namespace fs = std::filesystem;
using tape_sort::tape::FileTape;

static const std::string TEST_FILE = "test_tape.dat";

class FileTapeTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream out(TEST_FILE, std::ios::binary);

        std::vector<int32_t> data = {10, 20, 30, 40, 50};
        out.write(
            reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size() * sizeof(int32_t))
        );
    }

    void TearDown() override {
        fs::remove(TEST_FILE);
    }
};

TEST_F(FileTapeTest, ReadsFirstElement) {
    FileTape tape(TEST_FILE);

    int32_t value = tape.read();
    EXPECT_EQ(value, 10);
}

TEST_F(FileTapeTest, SequentialRead) {
    FileTape tape(TEST_FILE);

    EXPECT_EQ(tape.read(), 10);

    tape.move_right();
    EXPECT_EQ(tape.read(), 20);

    tape.move_right();
    EXPECT_EQ(tape.read(), 30);

    tape.move_right();
    EXPECT_EQ(tape.read(), 40);
}

TEST_F(FileTapeTest, MoveRightChangesPosition) {
    FileTape tape(TEST_FILE);

    tape.move_right();
    int32_t value = tape.read();

    EXPECT_EQ(value, 20);
}

TEST_F(FileTapeTest, MoveLeftWorks) {
    FileTape tape(TEST_FILE);

    tape.move_right();
    tape.move_right();
    tape.move_left();

    EXPECT_EQ(tape.read(), 20);
}

TEST_F(FileTapeTest, RewindReturnsToStart) {
    FileTape tape(TEST_FILE);

    tape.move_right();
    tape.move_right();

    tape.rewind();

    EXPECT_EQ(tape.read(), 10);
}

TEST_F(FileTapeTest, CanMoveRight) {
    FileTape tape(TEST_FILE);

    EXPECT_TRUE(tape.can_move_right());

    for (int i = 0; i < 4; i++) {
        tape.move_right();
    }

    EXPECT_FALSE(tape.can_move_right());
}

TEST_F(FileTapeTest, CanMoveLeft) {
    FileTape tape(TEST_FILE);

    EXPECT_FALSE(tape.can_move_left());

    tape.move_right();

    EXPECT_TRUE(tape.can_move_left());
}

TEST_F(FileTapeTest, WriteOverwritesData) {
    FileTape tape(TEST_FILE);

    tape.write(999);

    tape.rewind();

    EXPECT_EQ(tape.read(), 999);
}

TEST_F(FileTapeTest, FilePersistence) {
    {
        FileTape tape(TEST_FILE);
        tape.write(123);
    }

    FileTape tape2(TEST_FILE);
    EXPECT_EQ(tape2.read(), 123);
}

TEST_F(FileTapeTest, RandomMovesKeepConsistency) {
    FileTape tape(TEST_FILE);

    tape.move_right();
    tape.move_right();
    tape.move_left();
    tape.move_right();

    EXPECT_EQ(tape.read(), 30);
}
