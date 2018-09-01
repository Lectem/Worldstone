//
// Created by Lectem on 11/11/2016.
//
#pragma once

#include "IOBase.h"
#include "Stream.h"

namespace WorldStone
{

/**
 * @brief Base class to use/build archives.
 *
 * Archives are file/resource containers that provide basic I/O support.
 * It can be implemented as a basic filesystem to read disks, compressed archives, databases.
 * You can view this as an abstraction in the form of a directory
 *
 * @note Implementation does not need to be thread safe,
 *       so you have to handle race conditions yourself when manipulating multiple files
 *       from an archive, unless otherwise stated by the implementation
 *       You can however check if the implementation is thread-safe by calling isThreadSafe
 */
class Archive : public IOBase
{
protected:
    virtual bool load()      = 0;
    virtual bool is_loaded() = 0;
    virtual bool unload()    = 0;

public:
    Archive()               = default;
    Archive(const Archive&) = delete;
    Archive& operator=(const Archive&) = delete;
    virtual ~Archive();

    virtual bool exists(const Path& filePath)    = 0;
    virtual StreamPtr open(const Path& filePath) = 0;
    virtual bool isThreadSafe() { return false; }
};
}
