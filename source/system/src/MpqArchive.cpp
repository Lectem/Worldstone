//
// Created by Lectem on 11/11/2016.
//

#include "MpqArchive.h"
#include <StormLib.h>
#include <fmt/format.h>
#include <type_traits>

namespace WorldStone
{

MpqArchive::MpqArchive(const Archive::path& MpqFileName) : mpqFileName(MpqFileName)
{
    static_assert(std::is_same<MpqArchive::HANDLE, ::HANDLE>(),
                  "Make sure we correctly defined HANDLE type");
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
    return std::make_unique<MpqFileStream>(*this, filePath);
}

MpqFileStream::MpqFileStream(MpqArchive& archive, const path& filename)
{
    open(archive, filename);
}

MpqFileStream::~MpqFileStream()
{
    close();
}

bool MpqFileStream::open(MpqArchive& archive, const path& filename)
{
    if (!SFileOpenFileEx(archive.getInternalHandle(), filename.c_str(), 0, &file))
        setstate(failbit);
    return good();
}

bool MpqFileStream::close()
{
    if (!(file && SFileCloseFile(file))) setstate(failbit);
    return good();
}

size_t MpqFileStream::read(void* buffer, size_t size, size_t count)
{
    DWORD readBytes = 0;
    if (!SFileReadFile(file, buffer, static_cast<DWORD>(size * count), &readBytes, NULL)) {
        const DWORD lastError = GetLastError();
        if (lastError == ERROR_HANDLE_EOF)
            setstate(eofbit);
        else
        {
            setstate(failbit);
            return 0;
        }
    }
    return readBytes;
}

long MpqFileStream::tell()
{
    const DWORD size = SFileSetFilePointer(file, 0, NULL, FILE_CURRENT);
    if (size == SFILE_INVALID_SIZE) setstate(failbit);
    return static_cast<long>(size);
}

// We assume there's no need for conversion, which is probably wrong on some platforms
static_assert(MpqFileStream::beg == FILE_BEGIN, "");
static_assert(MpqFileStream::cur == FILE_CURRENT, "");
static_assert(MpqFileStream::end == FILE_END, "");
bool MpqFileStream::seek(long offset, Stream::seekdir origin)
{
    if (SFileSetFilePointer(file, static_cast<LONG>(offset), nullptr, origin) == SFILE_INVALID_SIZE)
        setstate(failbit);
    return good();
}
}
