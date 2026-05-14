#include "tape/delay_tape_decorator.hpp"

#include <thread>
#include <chrono>

using namespace tape_sort::tape;

DelayTapeDecorator::DelayTapeDecorator(ITape& tape, const config::TapeConfig& config)
    : tape_(tape)
    , config_(config)
{}

void DelayTapeDecorator::sleep_ms(int ms) {
    if (ms > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int32_t DelayTapeDecorator::read() {
    sleep_ms(config_.read_delay_ms);
    return tape_.read();
}

void DelayTapeDecorator::write(int32_t value) {
    sleep_ms(config_.write_delay_ms);
    tape_.write(value);
}

void DelayTapeDecorator::move_right() {
    sleep_ms(config_.move_delay_ms);
    tape_.move_right();
}

void DelayTapeDecorator::move_left() {
    sleep_ms(config_.move_delay_ms);
    tape_.move_left();
}

void DelayTapeDecorator::rewind() {
    sleep_ms(config_.rewind_delay_ms);
    tape_.rewind();
}

bool DelayTapeDecorator::is_bof() const {
    return tape_.is_bof();
}

bool DelayTapeDecorator::is_eof() const {
    return tape_.is_eof();
}

size_t DelayTapeDecorator::size() const {
    return tape_.size();
}

size_t DelayTapeDecorator::position() const {
    return tape_.position();
}
