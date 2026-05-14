#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>

#include "tape/delay_tape_decorator.hpp"

using namespace tape_sort::tape;
using namespace tape_sort::config;

namespace { // helper

class FakeTape : public ITape {
public:
    explicit FakeTape(std::vector<int32_t> d) : data(std::move(d)) {}

    int32_t read() override { return data[pos]; }
    void write(int32_t value) override { data[pos] = value; }

    void move_right() override { if (pos + 1 < data.size()) ++pos; }
    void move_left() override { if (pos > 0) --pos; }
    void rewind() override { pos = 0; }

    bool can_move_right() const override { return pos + 1 < data.size(); }
    bool can_move_left() const override { return pos > 0; }

private:
    std::vector<int32_t> data;
    size_t pos = 0;
};

auto now_ms() {
    return std::chrono::steady_clock::now();
}

long elapsed_ms(const std::chrono::steady_clock::time_point& start,
                const std::chrono::steady_clock::time_point& end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

} // namespace

TEST(DelayTapeDecoratorTest, ReadDoesNotChangeLogic) {
    FakeTape base({10, 20, 30});

    TapeConfig cfg{0, 0, 0, 0};
    DelayTapeDecorator tape(base, cfg);

    EXPECT_EQ(tape.read(), 10);

    tape.move_right();
    EXPECT_EQ(tape.read(), 20);
}

TEST(DelayTapeDecoratorTest, WriteAffectsUnderlyingTape) {
    FakeTape base({10, 20, 30});

    TapeConfig cfg{0, 0, 0, 0};
    DelayTapeDecorator tape(base, cfg);

    tape.write(999);
    EXPECT_EQ(tape.read(), 999);
}

TEST(DelayTapeDecoratorTest, MoveOperationsWorkCorrectly) {
    FakeTape base({10, 20, 30});

    TapeConfig cfg{0, 0, 0, 0};
    DelayTapeDecorator tape(base, cfg);

    EXPECT_EQ(tape.read(), 10);

    tape.move_right();
    EXPECT_EQ(tape.read(), 20);

    tape.move_left();
    EXPECT_EQ(tape.read(), 10);
}

TEST(DelayTapeDecoratorTest, RewindWorks) {
    FakeTape base({10, 20, 30});

    TapeConfig cfg{0, 0, 0, 0};
    DelayTapeDecorator tape(base, cfg);

    tape.move_right();
    tape.move_right();

    tape.rewind();

    EXPECT_EQ(tape.read(), 10);
}

TEST(DelayTapeDecoratorTest, CanMoveDelegation) {
    FakeTape base({10, 20});

    TapeConfig cfg{0, 0, 0, 0};
    DelayTapeDecorator tape(base, cfg);

    EXPECT_TRUE(tape.can_move_right());
    EXPECT_FALSE(tape.can_move_left());

    tape.move_right();

    EXPECT_FALSE(tape.can_move_right());
    EXPECT_TRUE(tape.can_move_left());
}

TEST(DelayTapeDecoratorTest, ReadDelayIsApplied) {
    FakeTape base({10});

    TapeConfig cfg{
        .read_delay_ms = 80,
        .write_delay_ms = 0,
        .move_delay_ms = 0,
        .rewind_delay_ms = 0
    };

    DelayTapeDecorator tape(base, cfg);

    auto start = now_ms();
    tape.read();
    auto end = now_ms();

    EXPECT_GE(elapsed_ms(start, end), 80);
}

TEST(DelayTapeDecoratorTest, WriteDelayIsApplied) {
    FakeTape base({10});

    TapeConfig cfg{
        .read_delay_ms = 0,
        .write_delay_ms = 80,
        .move_delay_ms = 0,
        .rewind_delay_ms = 0
    };

    DelayTapeDecorator tape(base, cfg);

    auto start = now_ms();
    tape.write(123);
    auto end = now_ms();

    EXPECT_GE(elapsed_ms(start, end), 80);
}

TEST(DelayTapeDecoratorTest, MoveDelayIsApplied) {
    FakeTape base({10, 20});

    TapeConfig cfg{
        .read_delay_ms = 0,
        .write_delay_ms = 0,
        .move_delay_ms = 80,
        .rewind_delay_ms = 0
    };

    DelayTapeDecorator tape(base, cfg);

    auto start = now_ms();
    tape.move_right();
    auto end = now_ms();

    EXPECT_GE(elapsed_ms(start, end), 80);
}

TEST(DelayTapeDecoratorTest, RewindDelayIsApplied) {
    FakeTape base({10, 20});

    TapeConfig cfg{
        .read_delay_ms = 0,
        .write_delay_ms = 0,
        .move_delay_ms = 0,
        .rewind_delay_ms = 80
    };

    DelayTapeDecorator tape(base, cfg);

    tape.move_right();

    auto start = now_ms();
    tape.rewind();
    auto end = now_ms();

    EXPECT_GE(elapsed_ms(start, end), 80);
}

TEST(DelayTapeDecoratorTest, ZeroDelaysAreFast) {
    FakeTape base({10, 20, 30});

    TapeConfig cfg{0, 0, 0, 0};
    DelayTapeDecorator tape(base, cfg);

    auto start = now_ms();
    tape.read();
    tape.write(111);
    tape.move_right();
    auto end = now_ms();

    EXPECT_LT(elapsed_ms(start, end), 20);
}
