//
// Created by Lectem on 12/11/2016.
//

#pragma once

#include <ios>
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
    using path    = std::string;
    using iostate = std::ios_base::iostate;

protected:
    static const iostate goodbit = std::ios_base::goodbit;
    static const iostate badbit  = std::ios_base::badbit;
    static const iostate failbit = std::ios_base::failbit;

    iostate _state = goodbit;

    void setstate(iostate state) { _state = _state | state; }

public:
    explicit operator bool() const { return !fail(); }
    bool operator!() const { return fail(); }
    bool good() const { return _state == goodbit; }
    bool fail() const { return (_state & (badbit | failbit)) != 0; }
    bool bad() const { return (_state & badbit) != 0; }
};
}
