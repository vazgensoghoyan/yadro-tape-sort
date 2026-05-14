#pragma once // tape/interface_tape.hpp

#include <cstddef>
#include <cstdint>

namespace tape_sort::tape {

class ITape {
public:
    virtual ~ITape() = default;

    virtual int32_t read() = 0;
    virtual void write(int32_t value) = 0;

    virtual void move_right() = 0;
    virtual void move_left() = 0;

    virtual void rewind() = 0; // перемотка ленты в начало

    virtual bool can_move_right() const = 0;
    virtual bool can_move_left() const = 0;
};

} // namespace tape_sort::tape
