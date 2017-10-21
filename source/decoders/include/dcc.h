#pragma once

#include <stdint.h>
#include <FileStream.h>
#include <memory>
#include <type_traits>
#include <vector>
#include "palette.h"

namespace WorldStone
{
/**
 * @brief Decoder for the DCC image format
 *
 * The DCC format is a compressed sprite image format.
 * It can store multiple directions and frames in a single file.
 * A palette is used to generate the colors, but the file itself doesn't have any information about
 * the palette to use, it is up to the user to know which one to use. @see WorldStone::Palette
 *
 * This format is usually used with COF files, which describe the animations and blending of
 * multiple DCC files.
 */
class DCC
{
public:
    /// The header of the file, contains global information about the encoded image
    struct Header
    {
        uint8_t signature;      ///< Magic number for DCC files, must be 0x74
        uint8_t version;        ///< DCC major version, usually 6
        uint8_t directions;     ///< Number of directions in this file, max 32
        uint8_t frames_per_dir; ///< Frames number for each direction(0-255? Max seen in-game is 200)
        uint8_t  padding0[3];   ///< Some padding, might actually still be frames_per_dir
        uint32_t tag;           ///< Seems to be always 1 ?

        /// Size of the decoded image in bytes
        ///(outsize_coded for all directions) + directions*frames_per_dir*4 + 24
        uint32_t final_dc6_size;
    };

    /**Represents a direction header
     * @note While the struct uses bitfields with the same sizes as the DCC format,
     *       it is only for documenation purposes, do not try to fwrite/fread it.
     *       (bitfields layout is implementation defined)
     */
    struct DirectionHeader
    {
        // clang-format off
        uint32_t outsize_coded;
        bool compressColorEncoding   : 1;
        bool compressEqualCells      : 1;
        uint32_t variable0_bits      : 4;
        uint32_t width_bits          : 4;
        uint32_t height_bits         : 4;
        uint32_t xoffset_bits        : 4;
        uint32_t yoffset_bits        : 4;
        uint32_t optional_bytes_bits : 4;
        uint32_t coded_bytes_bits    : 4;
        // clang-format on
    };

    struct FrameHeader
    {
        uint32_t variable0;
        uint32_t width;
        uint32_t height;
        int32_t  xoffset;
        int32_t  yoffset;
        uint32_t optional_bytes;
        uint32_t coded_bytes;
        bool     frame_bottom_up;
    };

    struct Direction
    {
        DirectionHeader header;
        std::vector<FrameHeader> frameHeaders;
    };

    /** An array that maps an encoded 4-bit size to the real size in bits.
     *  The values are { 0, 1, 2, 4, 6, 8, 10, 12, 14, 16, 20, 24, 26, 28, 30, 32 }
     */
    static constexpr unsigned bitsWidthTable[16] = {0,  1,  2,  4,  6,  8,  10, 12,
                                                    14, 16, 20, 24, 26, 28, 30, 32};

protected:
    StreamPtr stream = nullptr;
    Header    header;
    /* Offset of each direction header in the file, follows the @ref Header
     * Actually olds directions+1 values, the last one being the size of the file
     * It lets us compute the size of a direction by substracting the next offset by the current
     * one.
     */
    std::vector<uint32_t> directionsOffsets;
    std::vector<uint32_t> framePointers;

    size_t getDirectionSize(uint32_t dirIndex);

public:
    bool Decode(const char* filename);
    bool Decode(StreamPtr&& streamPtr);

    /// Resets the decoder and frees resources
    void Reset() { *this = DCC{}; }

    bool extractHeaderAndOffsets();
    bool readDirection(Direction& outDir, uint32_t dirIndex);

    const Header& getHeader() { return header; }
};
} // namespace WorldStone
