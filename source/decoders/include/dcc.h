#pragma once

#include <stdint.h>
#include <AABB.h>
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
 * @test{Decoders,DCC_BaalSpirit}
 */
class DCC
{
    using Extents = AABB<int32_t>;

public:
    /// The header of the file, contains global information about the encoded image
    struct Header
    {
        uint8_t  signature;    ///< Magic number for DCC files, must be 0x74
        uint8_t  version;      ///< DCC major version, usually 6
        uint8_t  directions;   ///< Number of directions in this file, max 32
        uint8_t  framesPerDir; ///< Frames number for each direction(0-255? Max seen in-game is 200)
        uint8_t  padding0[3];  ///< Some padding, might actually still be framesPerDir
        uint32_t tag;          ///< Seems to be always 1 ?

        /// Size of the decoded image in bytes
        ///(outsizeCoded for all directions) + directions*framesPerDir*4 + 24
        uint32_t finalDc6Size;
    };

    /** Represents a direction header.
     *
     * Some fields encode the size of various FrameHeader members using DCC::bitsWidthTable.
     * @note While the struct uses bitfields with the same sizes as the DCC format,
     *       it is only for documenation purposes, do not try to fwrite/fread it.
     *       (bitfields layout is implementation defined)
     *
     */
    struct DirectionHeader
    {
        // clang-format off
        uint32_t outsizeCoded;
        bool hasRawPixelEncoding   : 1; ///< Do we have a direction supporting the raw pixel encoding ?
        bool compressEqualCells    : 1; ///< Do we have a stream for the equal cells optimization ?
        uint32_t variable0Bits     : 4; ///< Endcoded size in bits of FrameHeader::variable0
        uint32_t widthBits         : 4; ///< Endcoded size in bits of FrameHeader::width
        uint32_t heightBits        : 4; ///< Endcoded size in bits of FrameHeader::height
        uint32_t xoffsetBits       : 4; ///< Endcoded size in bits of FrameHeader::xoffset
        uint32_t yoffsetBits       : 4; ///< Endcoded size in bits of FrameHeader::yoffset
        uint32_t optionalBytesBits : 4; ///< Endcoded size in bits of FrameHeader::optionalBytes
        uint32_t codedBytesBits    : 4; ///< Endcoded size in bits of FrameHeader::codedBytes
        // clang-format on
    };

    struct FrameHeader
    {
        ///@name Values in file
        ///@{
        uint32_t variable0;
        uint32_t width;
        uint32_t height;
        int32_t  xoffset;
        int32_t  yoffset;
        uint32_t optionalBytes;
        uint32_t codedBytes;
        bool     frameBottomUp;
        ///@}

        /** Extent of this frame image in the pixel buffer.
         * Boundaries are included
         */
        Extents extents;
    };

    struct Direction
    {
        ///@name Values in file
        ///@{
        DirectionHeader header;
        std::vector<FrameHeader> frameHeaders;
        ///@}

        Extents extents;

        ///@note Requires that the frame extents are already computed
        void computeDirExtents()
        {
            extents.initializeForExtension();
            for (const FrameHeader& frame : frameHeaders)
                extents.extend(frame.extents);
        }
    };

    /** An array that maps an encoded 4-bit size to the real size in bits.
     *  The values are { 0, 1, 2, 4, 6, 8, 10, 12, 14, 16, 20, 24, 26, 28, 30, 32 }
     */
    static constexpr unsigned bitsWidthTable[16] = {0,  1,  2,  4,  6,  8,  10, 12,
                                                    14, 16, 20, 24, 26, 28, 30, 32};

protected:
    StreamPtr stream = nullptr;
    Header    header;
    /** Offset of each direction header in the file, follows the @ref Header.
     * Actually holds directions+1 values, the last one being the size of the file
     * It lets us compute the size of a direction by substracting the next offset by the current
     * one.
     */
    std::vector<uint32_t> directionsOffsets;
    std::vector<uint32_t> framePointers;

    size_t getDirectionSize(uint32_t dirIndex);

    //    void decodeDirectionStage2(const Direction& dir);

public:
    bool decode(const char* filename);
    bool decode(StreamPtr&& streamPtr);

    /// Resets the decoder and frees resources
    void reset() { *this = DCC{}; }

    bool extractHeaderAndOffsets();

    bool readDirection(Direction& outDir, uint32_t dirIndex);

    const Header& getHeader() const { return header; }
};
} // namespace WorldStone
