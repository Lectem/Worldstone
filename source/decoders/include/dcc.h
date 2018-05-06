/**@file dcc.h
 * Implementation of a DCC file decoder
 */
#pragma once

#include <stdint.h>
#include <FileStream.h>
#include <Vector.h>
#include <memory>
#include <type_traits>
#include "AABB.h"
#include "ImageView.h"
#include "palette.h"

namespace WorldStone
{
// clang-format off
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
 *
 * Layout of a DC6 file:
 * | Name                       | Type                         | Size in bytes                               | Offsets                   |
 * | -------------------------- | ---------------------------- | ------------------------------------------- | ------------------------- |
 * | header                     | DC6::Header                  | 24                                          | 0x00                      |
 * | framePointers              | uint32_t[dirs][framesPerDir] | 4 * header.directions * header.framesPerDir | 0x18                      |
 * | frameHeader[0]             | DC6::FrameHeader             | 32                                          | framePointers[0]          |
 * | frameData[0]               | uint8_t[frameHeader.length]  | frameHeader[0].length                       | ^                         |
 * | termination[0]             | uint8_t[3]                   | 3                                           | ^                         |
 * | ... other frames ...       ||||
 * | frameHeader[totalFrames-1] | DC6::FrameHeader             | 32                                          | framePointers[nbFrames-1] |
 * | frameData[totalFrames-1]   | uint8_t[frameHeader.length]  | frameHeader[nbFrames-1].length              | ^                         |
 * | termination[totalFrames-1] | uint8_t[3]                   | 3                                           | ^                         |
 *
 * @test{Decoders,DCC_BaalSpirit}
 * @test{Decoders,DCC_CRHDBRVDTHTH}
 * @test{Decoders,DCC_BloodSmall01}
 * @test{Decoders,DCC_HZTRLITA1HTH}
 */
// clang-format on
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
        uint32_t xOffsetBits       : 4; ///< Endcoded size in bits of FrameHeader::xoffset
        uint32_t yOffsetBits       : 4; ///< Endcoded size in bits of FrameHeader::yoffset
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
        int32_t  xOffset;
        int32_t  yOffset;
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
        DirectionHeader     header;
        Vector<FrameHeader> frameHeaders;
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
    Vector<uint32_t> directionsOffsets;
    Vector<uint32_t> framePointers;

    size_t getDirectionSize(uint32_t dirIndex);

    bool extractHeaderAndOffsets();

public:
    /**Start decoding the stream and preparing data.
     * @return true on success
     *
     * Prepares the decoder to read the frames using @ref readDirection.
     * Basically calls extractHeaderAndOffsets, so that you can call @ref getHeader.
     */
    bool initDecoder(StreamPtr&& streamPtr);

    /// Resets the decoder and frees resources
    void reset() { *this = DCC{}; }

    /**Decodes a direction of the file into memory.
     * @param outDir   Will hold the Direction information obtained during decoding.
     * @param dirIndex The number of the direction in the file.
     * @param imgProvider The image provider to be used when allocating frames data.
     * @return true on success
     *
     * Use @ref getHeader and the @ref Header::directions member to know how many directions are in
     * the file. The frames will be allocated+decoded in order, hence using the images allocated
     * using the image provider will be in the order of the file.
     */
    bool readDirection(Direction& outDir, uint32_t dirIndex, IImageProvider<uint8_t>& imgProvider);

    /// Returns the header of the file read by extractHeaderAndOffsets
    const Header& getHeader() const { return header; }
};
} // namespace WorldStone
