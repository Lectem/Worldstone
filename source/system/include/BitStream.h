//
// Created by Lectem on 12/11/2016.
//

#pragma once

#include <stdint.h>
#include <SystemUtils.h>
#include <algorithm>
#include <assert.h>
#include <climits>
#include <type_traits>
#include "IOBase.h"
namespace WorldStone
{

/**
 * @brief Provides access to a variable bitsize values stream
 *
 * The bitstream operates as a view on raw memory to get values of different sizes.
 * This assumes that the data is ordered in a little endian fashion,
 * and signed values are encoded using 2's complement.
 *
 * @note This is basicly the bitstream format used by the @ref DCC format of Diablo 2
 * @note Does not inherit from @ref IStream to avoid confusion since size is in bits and not bytes.
 * Use @ref MemoryStream instead.
 * @warning As this class acts as a view, the buffer must outlive the usage of this class.
 * @todo Add some bounds checking and set io flags on error
 * @todo Implement a MemoryStream class
 * @test{System,RO_bitstream}
 */

class BitStream : public IOBase
{
    using byte = char;
    const byte* buffer = nullptr;
    size_t bufferSize = 0;
    size_t currentBitPosition = 0;

public:
    /// Creates a bitstream from raw memory
    BitStream(const void* inputBuffer, size_t sizeInBytes, size_t bitPosition = 0)
        : buffer(static_cast<const byte*>(inputBuffer)),
          bufferSize(sizeInBytes),
          currentBitPosition(bitPosition)
    {
        assert(currentBitPosition * CHAR_BIT <= sizeInBytes);
    }

    /// Returns the current position in bits
    size_t tell() { return currentBitPosition; }
    /// Set the current position, in bits
    void setPosition(size_t newPosition) { currentBitPosition = newPosition; }
    /// Skips the next nbBits bits
    void skip(size_t nbBits)
    {
        assert(currentBitPosition + nbBits <= sizeInBits());
        currentBitPosition += nbBits;
    }

    void alignToByte()
    {
        // Remove the last 3 bytes to be a multiple of 8, but add 7 before so that we round up
        currentBitPosition = (currentBitPosition + size_t(7)) & (~size_t(0x7));
    }

    /** Returns the total size of the current stream buffer in bits
     * @note This will always return a multiple of the size of a byte in bits.
     */
    size_t sizeInBits() { return bufferSize * sizeof(char) * CHAR_BIT; }
    /// Returns the total size of the current stream buffer in bytes
    size_t sizeInBytes() { return bufferSize; }

    /** Reads a single bit from the stream */
    bool readBool()
    {
        size_t currentBytesPos     = currentBitPosition / 8;
        size_t bitPosInCurrentByte = currentBitPosition % 8;
        int    mask                = (1 << bitPosInCurrentByte);
        currentBitPosition++;
        return (buffer[currentBytesPos] & mask) == mask;
    }
    /** Reads a single bit from the stream (uint32_t version) */
    uint32_t readBit() { return uint32_t(readBool()); }

    /** Reads an unsigned value of variable bit size
     * @tparam NbBits The number of bits to read from the stream
     * @tparam RetType The type of the value to return, must be at least NbBits bits big.
     * @note This function has not been optimized.
     *       We could do so by having special cases based on NbBits
     *       or simply keeping currentBytesPos/bitPosInCurrentByte in the class.
     *       But is it worth it ?
     */
    template<unsigned NbBits, typename RetType = uint32_t>
    RetType           readUnsigned()
    {
        static_assert(std::is_unsigned<RetType>::value, "You must return an unsigned type !");
        static_assert(NbBits <= sizeof(RetType) * CHAR_BIT, "RetType is too small!");
        RetType value           = 0;
        size_t  curBytesPos     = currentBitPosition / 8;
        size_t  bitPosInCurByte = currentBitPosition % 8;
        currentBitPosition += NbBits;
        // The following variable is needed because we are reading a little endian input.
        size_t curDestBitPosition = 0;
        while (curDestBitPosition < NbBits)
        {
            // How many bits we can read in this byte ?
            const size_t bytesToReadInCurByte =
                std::min(CHAR_BIT - bitPosInCurByte, NbBits - curDestBitPosition);
            const RetType mask = RetType((1U << bytesToReadInCurByte) - 1U);
            // The bits we got from the current byte
            const RetType inBits = (RetType(buffer[curBytesPos++]) >> bitPosInCurByte) & mask;
            // Place them at the right position
            value |= inBits << curDestBitPosition;
            curDestBitPosition += bytesToReadInCurByte;
            // Next time we start at the beginning of the byte, perhaps we could optimize by
            // Treating the first byte as a special case ?
            bitPosInCurByte = 0;
        }
        return value;
    }
    /** Return 0u, used for member function tables as replacement for BitStream::readUnsigned<0> */
    uint32_t read0Bits() { return 0u; }

    /** Reads an signed value of variable bit size. Values are using 2's complement.
     * @tparam NbBits The number of bits to read from the stream
     */
    template<unsigned NbBits, typename std::enable_if<(NbBits > 1u)>::type* = nullptr>
    int32_t           readSigned()
    {
        return Utils::signExtend<int32_t, NbBits>(readUnsigned<NbBits>());
    }

    /** Return 0 if reading less than 2 bits
     * @note This specialization is only available because sometimes we want to read a number of
     * bits choosen at runtime and use a method pointers table. But really, it'd be best to just not
     * call it.
     */
    template<unsigned NbBits, typename std::enable_if<(NbBits <= 1u)>::type* = nullptr>
    int32_t           readSigned()
    {
        currentBitPosition += NbBits;
        return 0;
    }
};
}
