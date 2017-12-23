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

class BitStreamView : public IOBase
{
    using byte                     = unsigned char;
    size_t      size               = 0;       ///< Size of the bitstream in bits
    size_t      firstBitOffset     = 0;       ///< Position of the first bit in the first byte
    size_t      currentBitPosition = 0;       ///< Current absolute position in the buffer, in bits
    const byte* buffer             = nullptr; ///< The buffer we are reading from

public:
    BitStreamView() = default;
    /// Creates a bitstream from raw memory
    BitStreamView(const void* inputBuffer, size_t sizeInBits, size_t firstBitOffsetInBuffer = 0)
        : size(sizeInBits),
          firstBitOffset(firstBitOffsetInBuffer),
          currentBitPosition(firstBitOffsetInBuffer),
          buffer(static_cast<const byte*>(inputBuffer))
    {
        assert(firstBitOffset + size <= bufferSizeInBits());
    }

    BitStreamView createSubView(size_t newbufferSizeInBits) const
    {
        // 0Bits is a special case as it should always have a size of 0
        if (newbufferSizeInBits == 0) return {};
        const size_t curBytesPos     = currentBitPosition / CHAR_BIT;
        const size_t bitPosInCurByte = currentBitPosition % CHAR_BIT;
        assert(tell() + newbufferSizeInBits <= size);
        assert(currentBitPosition + newbufferSizeInBits <= bufferSizeInBits());
        return {buffer + curBytesPos, newbufferSizeInBits, bitPosInCurByte};
    }

    /// Returns the current position in the stream in bits
    size_t tell() const { return currentBitPosition - firstBitOffset; }
    /// Set the current position, in bits
    void setPosition(size_t newPosition)
    {
        assert(newPosition >= 0_z && newPosition < size);
        currentBitPosition = newPosition + firstBitOffset;
    }
    /// Returns the current position in the buffer (ignoring the first bit position) in bits
    size_t bitPositionInBuffer() const { return currentBitPosition; }
    /// Skips the next nbBits bits
    void skip(size_t nbBits)
    {
        assert(currentBitPosition + nbBits < bufferSizeInBits());
        currentBitPosition += nbBits;
    }

    void alignToByte()
    {
        // Remove the last 3 bytes to be a multiple of 8, but add 7 before so that we round up
        currentBitPosition = (currentBitPosition + 7_z) & (~0x7_z);
    }

    size_t bufferSizeInBytes() const { return (size + firstBitOffset + 7) / CHAR_BIT; }
    /** Returns the total size of the current stream buffer in bits
     * @note This will always return a multiple of the size of a byte in bits.
     */
    size_t bufferSizeInBits() const { return bufferSizeInBytes() * CHAR_BIT; }
    /// Returns the total size of the current stream buffer in bytes
    size_t sizeInBits() const { return size; }

    /** Reads a single bit from the stream */
    bool readBool()
    {
        const size_t currentBytesPos     = currentBitPosition / CHAR_BIT;
        const size_t bitPosInCurrentByte = currentBitPosition % CHAR_BIT;
        const int    mask                = (1 << bitPosInCurrentByte);
        currentBitPosition++;
        return (buffer[currentBytesPos] & mask) == mask;
    }
    /** Reads a single bit from the stream (uint32_t version) */
    uint32_t readBit() { return uint32_t(readBool()); }

    /** Reads an unsigned value of variable bit size
     * @tparam RetType The type of the value to return, must be at least NbBits bits big.
     * @param nbBits The number of bits to read from the stream
     */
    template<typename RetType = uint32_t>
    RetType readUnsigned(unsigned nbBits)
    {
        static_assert(std::is_unsigned<RetType>::value, "You must return an unsigned type !");
        RetType value           = 0;
        size_t  curBytesPos     = currentBitPosition / CHAR_BIT;
        size_t  bitPosInCurByte = currentBitPosition % CHAR_BIT;
        currentBitPosition += nbBits;
        // The following variable is needed because we are reading a little endian input.
        size_t curDestBitPosition = 0;
        while (curDestBitPosition < nbBits)
        {
            // How many bits we can read in this byte ?
            const size_t bitsToReadInCurByte =
                std::min(CHAR_BIT - bitPosInCurByte, nbBits - curDestBitPosition);
            const RetType mask = RetType((1U << bitsToReadInCurByte) - 1U);
            // The bits we got from the current byte
            const RetType inBits = (RetType(buffer[curBytesPos++]) >> bitPosInCurByte) & mask;
            // Place them at the right position
            value |= inBits << curDestBitPosition;
            curDestBitPosition += bitsToReadInCurByte;
            // Next time we start at the beginning of the byte, perhaps we could optimize by
            // Treating the first byte as a special case ?
            bitPosInCurByte = 0;
        }
        return value;
    }

    uint8_t readUnsigned8OrLess(const int nbBits)
    {
        const size_t curBytesPos     = currentBitPosition / CHAR_BIT;
        const int    bitPosInCurByte = currentBitPosition % CHAR_BIT;
        currentBitPosition += size_t(nbBits);
        const uint16_t shortFromBuffer =
            buffer[curBytesPos] | uint16_t(buffer[curBytesPos + 1] << CHAR_BIT);
        const unsigned mask  = 0xFF >> (CHAR_BIT - nbBits);
        const uint8_t  value = uint8_t((shortFromBuffer >> bitPosInCurByte) & mask);
        return value;
    }

    /** Return 0u, used for member function tables as replacement for BitStream::readUnsigned<0> */
    uint32_t read0Bits() { return 0u; }

    /** Reads an signed value of variable bit size. Values are using 2's complement.
     * @tparam NbBits The number of bits to read from the stream
     * @note The value is sign extended, hence for 1-bit values: 0b0 is 0 and 0b1 is -1.
     */
    template<unsigned NbBits, typename std::enable_if<(NbBits > 0)>::type* = nullptr>
    int32_t           readSigned()
    {
        return Utils::signExtend<int32_t, NbBits>(readUnsigned(NbBits));
    }

    /** Return 0 if reading less than 2 bits
     * @note This specialization is only available because sometimes we want to read a number of
     * bits choosen at runtime and use a method pointers table. But really, it'd be best to just not
     * call it.
     */
    template<unsigned NbBits, typename std::enable_if<(NbBits == 0)>::type* = nullptr>
    int32_t           readSigned()
    {
        currentBitPosition += NbBits;
        return 0;
    }
};
}
