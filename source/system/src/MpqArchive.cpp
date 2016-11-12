//
// Created by Lectem on 11/11/2016.
//

#include "MpqArchive.h"
#include <StormLib.h>
#include <fmt/format.h>

namespace WorldStone
{

MpqArchive::MpqArchive(const Archive::path& MpqFileName) : mpqFileName(MpqFileName)
{
    load();
}

MpqArchive::~MpqArchive()
{
    unload();
}

bool MpqArchive::load()
{
    if (mpqHandle) throw std::runtime_error("tried to reopen mpq archive");
    if (!SFileOpenArchive(mpqFileName.c_str(), 0, STREAM_FLAG_READ_ONLY, &mpqHandle))
        setstate(failbit);
    return good();
}

bool MpqArchive::is_loaded()
{
    return mpqHandle != nullptr;
}

bool MpqArchive::unload()
{
    if (!(mpqHandle && SFileCloseArchive(mpqHandle))) setstate(failbit);
    mpqHandle = nullptr;
    return good();
}

bool MpqArchive::exists(const path& filePath)
{
    return SFileHasFile(mpqHandle, filePath.c_str());
}

StreamPtr MpqArchive::open(const path& filePath)
{
    (void)filePath;
    return nullptr;
}
}
