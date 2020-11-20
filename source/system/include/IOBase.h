//
// Created by Lectem on 12/11/2016.
//

#pragma once

#include <string>

namespace WorldStone
{
/**
 * This class reuses the parts of std::ios API but doesn't provide heavy stream functionnality (ie.
 * no << and >> )
 * Derived classes must have RAII behaviour.
 */
class IOBase
{
public:
    using Path    = std::string;
    using iostate = int;

protected:
    static constexpr iostate goodbit = 0x0;
    static constexpr iostate eofbit  = 0x1;
    static constexpr iostate failbit = 0x2;
    static constexpr iostate badbit  = 0x4;
    iostate                  _state  = goodbit;

    void setstate(iostate state) { _state = _state | state; }

public:
    explicit operator bool() const { return !fail(); }
    bool     operator!() const { return fail(); }

    bool good() const { return _state == goodbit; }
    bool eof() const { return _state & eofbit; }
    bool fail() const { return (_state & (badbit | failbit)) != 0; }
    bool bad() const { return (_state & badbit) != 0; }
};
} // namespace WorldStone
