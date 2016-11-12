//
// Created by Lectem on 11/11/2016.
//
#pragma once

#include <memory>
#include "IOBase.h"

namespace WorldStone
{
class Stream : public IOBase
{

public:
    static const iostate eofbit = std::ios_base::eofbit;
    bool                 eof() const { return (_state & eofbit) != 0; }

    using seekdir            = std::ios_base::seekdir;
    static const seekdir beg = std::ios_base::beg;
    static const seekdir cur = std::ios_base::cur;
    static const seekdir end = std::ios_base::end;

    virtual size_t read(void* buffer, size_t size, size_t count) = 0;
    virtual long tell() = 0;
    virtual bool seek(long offset, seekdir origin) = 0;

    virtual ~Stream();
};

using StreamPtr = std::unique_ptr<Stream>;
}
