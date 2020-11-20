//
// Created by Lectem on 12/11/2016.
//

#include "FileStream.h"
#include <assert.h>

namespace WorldStone
{

FileStream::FileStream(const Path& filename) { open(filename); }

FileStream::~FileStream() { close(); }

bool FileStream::open(const Path& filename)
{
    file = fopen(filename.c_str(), "rb");
    if (!file) setstate(failbit);
    return good();
}

bool FileStream::close()
{
    if (!file || fclose(file)) setstate(failbit);
    file = nullptr;
    return good();
}

size_t FileStream::read(void* buffer, size_t size)
{
    assert(is_open());
    const size_t readSize = fread(buffer, sizeof(char), size, file);
    if (readSize != size)
    {
        if (feof(file)) { setstate(eofbit); }
        setstate(failbit);
    }
    return readSize;
}

long FileStream::tell()
{
    assert(is_open());
    return ftell(file);
}

// We assume there's no need for conversion, which is probably wrong on some platforms
static_assert(FileStream::beg == SEEK_SET, "");
static_assert(FileStream::cur == SEEK_CUR, "");
static_assert(FileStream::end == SEEK_END, "");
bool FileStream::seek(long offset, IStream::seekdir origin)
{
    assert(is_open());
    if (fseek(file, offset, origin) != 0) setstate(failbit);
    return good();
}

long FileStream::size()
{
    const long curPos = ftell(file);
    if (curPos == -1) setstate(failbit);
    seek(0, end);
    const long size = ftell(file);
    if (size == -1) setstate(failbit);
    // Reset the file position even if the previous commands failed. Do not fail if position was
    // invalid
    if (curPos != -1 && !seek(curPos, beg)) setstate(failbit);
    return size;
}

int FileStream::getc()
{
    const int c = fgetc(file);
    if (c == EOF) { setstate(failbit | eofbit); }
    return c;
}
} // namespace WorldStone
