#pragma once // tape/delay_tape_decorator.hpp

#include <memory>
#include <chrono>
#include <thread>

#include "tape/interface_tape.hpp"
#include "config/config_structs.hpp"

namespace tape_sort::tape {

class DelayTapeDecorator final : public ITape {
public:
    DelayTapeDecorator(ITape& tape, const config::TapeConfig& config);

    ~DelayTapeDecorator() override = default;

    int32_t read() override;
    void write(int32_t value) override;

    void move_right() override;
    void move_left() override;

    void rewind() override;

    bool can_move_right() const override;
    bool can_move_left() const override;

private:
    static void sleep_ms(int ms);

private:
    ITape& tape_;
    config::TapeConfig config_;
};

} // namespace tape_sort::tape
