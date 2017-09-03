//
// Created by Lectem on 12/11/2016.
//

#include "Archive.h"
#include "Stream.h"

/*
 * This file was created to avoid weak vtables (it provides a TU to store the vtables)
 */

namespace WorldStone
{
Archive::~Archive() {}
IStream::~IStream() {}

int IStream::getc()
{
    uint8_t val;
    this->read(&val, sizeof(char));
    if (good())
        return val;
    else
        return -1;
}
}
