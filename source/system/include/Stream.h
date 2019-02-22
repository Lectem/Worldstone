/**
 * @file Stream.h
 * @author Lectem
 * @date 11/11/2016.
 */
#pragma once

#include <memory>
#include "IOBase.h"

namespace WorldStone
{

/**
 * @brief An interface for a stream of data.
 *
 * While it can be used to abstract many types of streams, file streams are
 * usually created through an @see Archive.
 *
 * @test{System,RO_filestreams}
 */
class IStream : public IOBase
{
public:
    /// True if the end of the stream was reached during the last read operation
    bool                 eof() const { return (_state & eofbit) != 0; }

    enum seekdir
    {
        beg,
        cur,
        end
    };
    /**
     * Compute the size of the file.
     * @return the size of the file, or a negative value on error
     */
    virtual long size() = 0;
    /**
     * Read data from the stream.
     * @param buffer Pointer to a block of memory to fill. Must be at least 'size' bytes large.
     * @param size   Number of bytes to copy
     * @return       The number of bytes successfully read. If less than 'size', EOF was reached or
     * an error occured.
     */
    virtual size_t read(void* buffer, size_t size) = 0;

    template<typename T>
    bool readRaw(T& out)
    {
        static_assert(std::is_trivially_copyable<T>::value,
                      "The type must be trivially copyable to access it as byte storage");
        return sizeof(out) == read(&out, sizeof(out));
    }
    /**
     * Read one byte from the stream.
     * @return The next byte to read from the stream, or a negative value on failure.
     */
    virtual int getc();
    /**
     * Return the current position of the stream pointer
     * @return The offset in bytes to the currently pointed position. A negative value on failure.
     * @note The @ref seek method can be used to restore a position.
     */
    virtual long tell() = 0;
    /**
     * Change the pointer of the stream to a given position
     * @param offset The new position relative to the given reference position 'origin'.
     * @param origin Arbitrary reference to a position in the stream.
     * @return true on success, false if an error occured or an invalid position was given.
     * @see seekdir values : beg cur end
     */
    virtual bool seek(long offset, seekdir origin) = 0;

    virtual ~IStream();
};

using StreamPtr = std::unique_ptr<IStream>;
}
