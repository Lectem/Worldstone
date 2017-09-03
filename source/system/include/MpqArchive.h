//
// Created by Lectem on 11/11/2016.
//

#pragma once

#include <vector>
#include "Archive.h"
#include "Stream.h"

namespace WorldStone
{

/**
 * @brief A wrapper to manage MPQ archives
 */
class MpqArchive : public Archive
{
    using HANDLE = void*; // Do not expose stormlib
public:
    MpqArchive() { setstate(badbit); }
    MpqArchive(const char* MpqFileName);
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
 * @brief A file from a MpqArchive
 */
class MpqFileStream : public IStream
{
    using HANDLE = void*;

    HANDLE file = nullptr;

protected:
    MpqFileStream() = default; // Needed to make tests easier
public:
    MpqFileStream(MpqArchive& archive, const path& filename);
    bool open(MpqArchive& archive, const path& filename);
    bool is_open() const { return file != nullptr; }
    bool close();

    size_t read(void* buffer, size_t size) override;

    long tell() override;
    long size() override;

    /**
     * @warning offset value must fit in 32bits
     */
    bool seek(long offset, seekdir origin) override;

    ~MpqFileStream() override;
};
}
