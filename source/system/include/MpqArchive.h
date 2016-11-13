//
// Created by Lectem on 11/11/2016.
//

#pragma once

#include "Archive.h"

namespace WorldStone
{

/**
 * A wrapper to manage MPQ archives
 */
class MpqArchive : public Archive
{
    using HANDLE = void*; // Do not expose stormlib
public:
    MpqArchive(const path& MpqFileName);
    ~MpqArchive() override;

    bool exists(const path& filePath) override;
    StreamPtr open(const path& filePath) override;

    HANDLE getInternalHandle() { return mpqHandle; }

private:
    bool load() override;
    bool is_loaded() override;
    bool unload() override;

    path   mpqFileName;
    HANDLE mpqHandle = nullptr;
};

/**
 * A MPQ file stream
 *
 */
class MpqFileStream : public Stream
{
    using HANDLE = void*;

    HANDLE file = nullptr;

public:
    MpqFileStream(MpqArchive& archive, const path& filename);
    bool open(MpqArchive& archive, const path& filename);
    bool is_open() const { return file != nullptr; }
    bool close();

    size_t read(void* buffer, size_t size, size_t count) override;

    long tell() override;

    /**
     * @warning offset value must fit in 32bits
     */
    bool seek(long offset, seekdir origin) override;

    ~MpqFileStream() override;
};
}
