//
// Created by Lectem on 11/11/2016.
//

#include "MpqArchive.h"
#include <StormLib.h>
#include <fmt/format.h>
#include <type_traits>

namespace WorldStone
{

MpqArchive::MpqArchive(MpqArchive&& toMove) { *this = std::move(toMove); }

MpqArchive& MpqArchive::operator=(MpqArchive&& toMove)
{
    std::swap(mpqHandle, toMove.mpqHandle);
    std::swap(mpqFileName, toMove.mpqFileName);
    std::swap(_state, toMove._state);
    return *this;
}
MpqArchive::MpqArchive(const char* MpqFileName, const char* listFilePath) : mpqFileName(MpqFileName)
{
    static_assert(std::is_same<MpqArchive::HANDLE, ::HANDLE>(),
                  "Make sure we correctly defined HANDLE type");
    if (load() && listFilePath) { addListFile(listFilePath); }
}

MpqArchive::~MpqArchive() { unload(); }

void MpqArchive::addListFile(const char* listFilePath)
{
    if (!mpqHandle) return;
    if (SFileAddListFile(mpqHandle, listFilePath) != ERROR_SUCCESS) setstate(failbit);
}

std::vector<MpqArchive::Path> MpqArchive::findFiles(const Path& searchMask)
{
    if (!mpqHandle) return {};
    std::vector<Path> list;
    SFILE_FIND_DATA   findFileData;
    HANDLE findHandle = SFileFindFirstFile(mpqHandle, searchMask.c_str(), &findFileData, nullptr);
    if (!findHandle) return {};
    do
    {
        list.emplace_back(findFileData.cFileName);
    } while (SFileFindNextFile(findHandle, &findFileData));

    SFileFindClose(findHandle);
    return list;
}

bool MpqArchive::load()
{
    if (mpqHandle) throw std::runtime_error("tried to reopen mpq archive");
    if (!SFileOpenArchive(mpqFileName.c_str(), 0, STREAM_FLAG_READ_ONLY, &mpqHandle))
        setstate(failbit);
    return good();
}

bool MpqArchive::is_loaded() { return mpqHandle != nullptr; }

bool MpqArchive::unload()
{
    if (!(mpqHandle && SFileCloseArchive(mpqHandle))) setstate(failbit);
    mpqHandle = nullptr;
    return good();
}

bool MpqArchive::exists(const Path& filePath) { return SFileHasFile(mpqHandle, filePath.c_str()); }

StreamPtr MpqArchive::open(const Path& filePath)
{
    StreamPtr tmp = std::make_unique<MpqFileStream>(*this, filePath);
    return tmp->good() ? std::move(tmp) : nullptr;
}

MpqFileStream::MpqFileStream(MpqArchive& archive, const Path& filename) { open(archive, filename); }

MpqFileStream::~MpqFileStream() { close(); }

bool MpqFileStream::open(MpqArchive& archive, const Path& filename)
{
    if (!archive || !SFileOpenFileEx(archive.getInternalHandle(), filename.c_str(), 0, &file))
        setstate(failbit);
    return good();
}

bool MpqFileStream::close()
{
    if (!(file && SFileCloseFile(file))) setstate(failbit);
    file = nullptr;
    return good();
}

size_t MpqFileStream::read(void* buffer, size_t size)
{
    DWORD readBytes = 0;

    bool success = SFileReadFile(file, buffer, static_cast<DWORD>(size), &readBytes, nullptr);
    if (!success)
    {
        const DWORD lastError = GetLastError();
        if (lastError == ERROR_HANDLE_EOF) { setstate(eofbit); }
        setstate(failbit);
    }
    return readBytes;
}

long MpqFileStream::tell()
{
    const DWORD size = SFileSetFilePointer(file, 0, nullptr, FILE_CURRENT);
    if (size == SFILE_INVALID_SIZE) setstate(failbit);
    return static_cast<long>(size);
}

// We assume there's no need for conversion, which is probably wrong on some platforms
static_assert(MpqFileStream::beg == FILE_BEGIN, "");
static_assert(MpqFileStream::cur == FILE_CURRENT, "");
static_assert(MpqFileStream::end == FILE_END, "");
bool MpqFileStream::seek(long offset, IStream::seekdir origin)
{
    if (SFileSetFilePointer(file, static_cast<LONG>(offset), nullptr, origin) == SFILE_INVALID_SIZE)
        setstate(failbit);
    return good();
}

long MpqFileStream::size()
{
    DWORD sizeLower32bits = SFileGetFileSize(file, nullptr);
    if (sizeLower32bits == SFILE_INVALID_SIZE) setstate(failbit);
    return static_cast<long>(sizeLower32bits);
}
} // namespace WorldStone
