//
// Created by Lectem on 12/11/2016.
//

#pragma once

#include <algorithm>
#include <climits>
#include <stdint.h>
#include <SystemUtils.h>
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
 */

class BitStream : public IOBase
{
    using byte = char;
    const byte* buffer = nullptr;
    size_t bufferSize = 0;
    size_t currentBitPosition = 0;

public:
    /// Creates a bitstream from raw memory
    BitStream(const byte* inputBuffer, size_t size) : buffer(inputBuffer), bufferSize(size) {}

    /// Returns the current position in bits
    size_t tell() { return currentBitPosition; }
    /// Set the current position, in bits
    void setPosition(size_t newPosition) { currentBitPosition = newPosition; }

    /** Returns the total size of the current stream buffer in bits
     * @note This will always return a multiple of the size of a byte in bits.
     */
    size_t sizeInBits() { return bufferSize * sizeof(char) * CHAR_BIT; }
    /// Returns the total size of the current stream buffer in bytes
    size_t sizeInBytes() { return bufferSize; }

    /** Reads an unsigned value of variable bit size
     * @tparam NbBits The number of bits to read from the stream
     * @note This function has not been optimized.
     *       We could do so by having special cases based on NbBits
     *       or simply keeping currentBytesPos/bitPosInCurrentByte in the class.
     *       But is it worth it ?
     */
    template<unsigned NbBits>
    uint32_t          readUnsigned()
    {
        uint32_t value               = 0;
        size_t   currentBytesPos     = currentBitPosition / 8;
        size_t   bitPosInCurrentByte = currentBitPosition % 8;
        currentBitPosition += NbBits;
        // The following variable is needed because we are reading a little endian input.
        size_t currentDestBitPosition = 0;
        while (currentDestBitPosition < NbBits)
        {
            // How many bits we can read in this byte ?
            size_t bytesToReadInCurrentByte =
                std::min(CHAR_BIT - bitPosInCurrentByte, NbBits - currentDestBitPosition);
            uint32_t mask = (1U << bytesToReadInCurrentByte) - 1;
            // The bits we got from the current byte
            uint32_t inBits = (uint32_t(buffer[currentBytesPos++]) >> bitPosInCurrentByte) & mask;
            // Place them at the right position
            value |= inBits << currentDestBitPosition;
            currentDestBitPosition += bytesToReadInCurrentByte;
            // Next time we start at the beginning of the byte, perhaps we could optimize by
            // Treating the first byte as a special case ?
            bitPosInCurrentByte = 0;
        }
        return value;
    }

    /** Reads an signed value of variable bit size. Values are using 2's complement.
     * @tparam NbBits The number of bits to read from the stream
     */
    template<unsigned NbBits>
    int32_t           readSigned()
    {
        return Utils::signExtend<int32_t, NbBits>(readUnsigned<NbBits>());
    }
};
}
