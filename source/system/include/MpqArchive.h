//
// Created by Lectem on 11/11/2016.
//

#pragma once

#include "Archive.h"
#include "Stream.h"
#include <vector>

namespace WorldStone
{

/**
 * A wrapper to manage MPQ archives
 */
class MpqArchive : public Archive
{
    using HANDLE = void*; // Do not expose stormlib
public:
    MpqArchive() { setstate(badbit); }
    MpqArchive(const path& MpqFileName);
    MpqArchive(MpqArchive&& toMove);
    MpqArchive& operator=(MpqArchive&& toMove);
    ~MpqArchive() override;

    bool exists(const path& filePath) override;
    StreamPtr open(const path& filePath) override;

    HANDLE getInternalHandle() { return mpqHandle; }

    void addListFile(const path& listFilePAth);
    std::vector<path> findFiles(const path& searchMask = "*");

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

    streamsize read(void* buffer, size_t size, size_t count) override;

    long tell() override;

    /**
     * @warning offset value must fit in 32bits
     */
    bool seek(long offset, seekdir origin) override;

    ~MpqFileStream() override;
};
}
