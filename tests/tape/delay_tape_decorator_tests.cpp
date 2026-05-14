#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <cstdint>

#include "tape/delay_tape_decorator.hpp"
#include "config/config_structs.hpp"

using namespace tape_sort::tape;
using namespace tape_sort::config;

namespace {

class MockTape final : public ITape {
public:
    int32_t read_value = 123;

    int read_calls = 0;
    int write_calls = 0;
    int move_right_calls = 0;
    int move_left_calls = 0;
    int rewind_calls = 0;

    bool bof = false;
    bool eof = false;

    size_t tape_size = 10;
    size_t tape_position = 1;

    int32_t read() override {
        ++read_calls;
        return read_value;
    }

    void write(int32_t value) override {
        ++write_calls;
        last_written_value = value;
    }

    void move_right() override {
        ++move_right_calls;
        ++tape_position;
    }

    void move_left() override {
        ++move_left_calls;

        if (tape_position > 0)
            --tape_position;
    }

    void rewind() override {
        ++rewind_calls;
        tape_position = 1;
    }

    bool is_bof() const override {
        return bof;
    }

    bool is_eof() const override {
        return eof;
    }

    size_t size() const override {
        return tape_size;
    }

    size_t position() const override {
        return tape_position;
    }

public:
    int32_t last_written_value = 0;
};

} // namespace

class DelayTapeDecoratorTest : public ::testing::Test {
protected:
    TapeConfig make_config(
        int read_ms = 0,
        int write_ms = 0,
        int move_ms = 0,
        int rewind_ms = 0)
    {
        return TapeConfig{
            .read_delay_ms = read_ms,
            .write_delay_ms = write_ms,
            .move_delay_ms = move_ms,
            .rewind_delay_ms = rewind_ms
        };
    }

protected:
    MockTape mock_tape_;
};

TEST_F(DelayTapeDecoratorTest, ReadDelegatesToWrappedTape)
{
    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    const auto value = tape.read();

    EXPECT_EQ(value, 123);
    EXPECT_EQ(mock_tape_.read_calls, 1);
}

TEST_F(DelayTapeDecoratorTest, WriteDelegatesToWrappedTape)
{
    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    tape.write(777);

    EXPECT_EQ(mock_tape_.write_calls, 1);
    EXPECT_EQ(mock_tape_.last_written_value, 777);
}

TEST_F(DelayTapeDecoratorTest, MoveRightDelegatesToWrappedTape)
{
    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    tape.move_right();

    EXPECT_EQ(mock_tape_.move_right_calls, 1);
    EXPECT_EQ(mock_tape_.position(), 2);
}

TEST_F(DelayTapeDecoratorTest, MoveLeftDelegatesToWrappedTape)
{
    mock_tape_.tape_position = 5;

    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    tape.move_left();

    EXPECT_EQ(mock_tape_.move_left_calls, 1);
    EXPECT_EQ(mock_tape_.position(), 4);
}

TEST_F(DelayTapeDecoratorTest, RewindDelegatesToWrappedTape)
{
    mock_tape_.tape_position = 8;

    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    tape.rewind();

    EXPECT_EQ(mock_tape_.rewind_calls, 1);
    EXPECT_EQ(mock_tape_.position(), 1);
}

TEST_F(DelayTapeDecoratorTest, IsBofDelegatesToWrappedTape)
{
    mock_tape_.bof = true;

    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    EXPECT_TRUE(tape.is_bof());
}

TEST_F(DelayTapeDecoratorTest, IsEofDelegatesToWrappedTape)
{
    mock_tape_.eof = true;

    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    EXPECT_TRUE(tape.is_eof());
}

TEST_F(DelayTapeDecoratorTest, SizeDelegatesToWrappedTape)
{
    mock_tape_.tape_size = 999;

    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    EXPECT_EQ(tape.size(), 999);
}

TEST_F(DelayTapeDecoratorTest, PositionDelegatesToWrappedTape)
{
    mock_tape_.tape_position = 42;

    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    EXPECT_EQ(tape.position(), 42);
}

TEST_F(DelayTapeDecoratorTest, ReadAppliesDelay)
{
    DelayTapeDecorator tape(
        mock_tape_,
        make_config(50, 0, 0, 0)
    );

    const auto start = std::chrono::steady_clock::now();

    tape.read();

    const auto end = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        );

    EXPECT_GE(elapsed.count(), 45);
}

TEST_F(DelayTapeDecoratorTest, WriteAppliesDelay)
{
    DelayTapeDecorator tape(
        mock_tape_,
        make_config(0, 50, 0, 0)
    );

    const auto start = std::chrono::steady_clock::now();

    tape.write(123);

    const auto end = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        );

    EXPECT_GE(elapsed.count(), 45);
}

TEST_F(DelayTapeDecoratorTest, MoveAppliesDelay)
{
    DelayTapeDecorator tape(
        mock_tape_,
        make_config(0, 0, 50, 0)
    );

    const auto start = std::chrono::steady_clock::now();

    tape.move_right();

    const auto end = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        );

    EXPECT_GE(elapsed.count(), 45);
}

TEST_F(DelayTapeDecoratorTest, RewindAppliesDelay)
{
    DelayTapeDecorator tape(
        mock_tape_,
        make_config(0, 0, 0, 50)
    );

    const auto start = std::chrono::steady_clock::now();

    tape.rewind();

    const auto end = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        );

    EXPECT_GE(elapsed.count(), 45);
}

TEST_F(DelayTapeDecoratorTest, ZeroDelayDoesNotBlockExecution)
{
    DelayTapeDecorator tape(
        mock_tape_,
        make_config()
    );

    const auto start = std::chrono::steady_clock::now();

    tape.read();
    tape.write(1);
    tape.move_right();
    tape.move_left();
    tape.rewind();

    const auto end = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        );

    EXPECT_LT(elapsed.count(), 20);
}
