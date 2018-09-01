/**
 * @file FileStream.h
 * @author Lectem
 */

#pragma once

#include "Stream.h"

namespace WorldStone
{

/**
 * @brief Wrapper around POSIX file io, same as ifstream but without iostream formatting
 */
class FileStream : public IStream
{
    FILE* file = nullptr;

public:
    FileStream(const Path& filename);
    ~FileStream() override;

    bool open(const Path& filename);
    bool is_open() const { return file != nullptr; }
    bool close();

    long tell() override;
    bool seek(long offset, seekdir origin) override;

    /** Computes the size of the file.
     * @return  The size of the file on success, -1 on error
     * @note    This will call tell/seek multiple times, hence can be slow
     * @warning If the current position is invalid, the new position might be EOF
     */
    long   size() override;
    size_t read(void* buffer, size_t size) override;
    int getc() override;
};
}
