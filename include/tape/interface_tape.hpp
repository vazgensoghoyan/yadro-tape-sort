#pragma once // tape/interface_tape.hpp

#include <cstddef>
#include <cstdint>

namespace tape_sort::tape {

// Логически у нас такой tape:
// BOF el_1 el_2 .. el_N EOF

// BOF: position() == 0
// EOF: position() == size() + 1

// read/write на EOF/BOF кидают исключение
// read/write сами головку не двигают

// при этом размер я предполагаю константный, не можем увеличивать ленту

// rewind перематывает на элемент (либо EOF если файл пустой)
// (логически надо перевести в состояние position() == 1 чтобы это заработало)

class ITape {
public:
    virtual ~ITape() = default;

    virtual int32_t read() = 0;
    virtual void write(int32_t value) = 0;

    virtual void move_right() = 0;
    virtual void move_left() = 0;

    virtual void rewind() = 0;

    virtual bool is_bof() const = 0;
    virtual bool is_eof() const = 0;

    virtual size_t size() const = 0;
    virtual size_t position() const = 0;
};

} // namespace tape_sort::tape
