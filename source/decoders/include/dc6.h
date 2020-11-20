#pragma once

#include <stdint.h>
#include <Stream.h>
#include <memory>
#include <type_traits>
#include <vector>
#include "Palette.h"

namespace WorldStone
{
// clang-format off
/**
 * @brief Decoder for the DC6 image format
 *
 * The DC6 (Diablo Cel 6) format is a compressed sprite image format.
 * It can store multiple directions and frames in a single file.
 * A palette is used to generate the colors, but the file itself doesn't have any information about
 * the palette to use, it is up to the user to know which one to use. @see WorldStone::Palette
 *
 * This format is mostly used for menu, items but also for some monsters (eg:Mephisto)
 * This is an update of diablo 1 Cel format
 *
 * Layout of a DC6 file:
 * | Name                       | Type                         | Size in bytes                               | Offsets                   |
 * | ---------------------------| ---------------------------- | ------------------------------------------- | ------------------------- |
 * | header                     | DC6::Header                  | 24                                          | 0x00                      |
 * | framePointers              | uint32_t[dirs][framesPerDir] | 4 * header.directions * header.framesPerDir | 0x18                      |
 * | frameHeader[0]             | DC6::FrameHeader             | 32                                          | framePointers[0]          |
 * | frameData[0]               | uint8_t[frameHeader.length]  | frameHeader[0].length                       | ^                         |
 * | skipColor[0]               | uint8_t[3]                   | 3                                           | ^                         |
 * | ... other frames ...       ||||
 * | frameHeader[totalFrames-1] | DC6::FrameHeader             | 32                                          | framePointers[nbFrames-1] |
 * | frameData[totalFrames-1]   | uint8_t[frameHeader.length]  | frameHeader[nbFrames-1].length              | ^                         |
 * | skipColor[totalFrames-1]   | uint8_t[3]                   | 3                                           | ^                         |
 *
 */
// clang-format on
class DC6
{
public:
    enum Flags
    {
        IsSerialized = 1 << 0, ///< Always set for files
        IsLoadedInHW = 1 << 1, ///< Used by the game to know if the file was loaded in the renderer
        Is24Bits = 1 << 2, ///< Used internally by the game for 24 to 8 bits per color conversion
    };

    struct Header
    {
        int32_t  version;      ///< DC6 major version, usually 6
        int32_t  flags;        ///< @ref DC6::Flags
        int32_t  format;       ///< Always 0 in the game files. 0=indexed 2=24bits
        uint8_t  skipColor[4]; ///< Skipped RGB color in D2CMP CelIteratePixels (unused in game)
        uint32_t directions;   ///< Number of directions in this file
        uint32_t framesPerDir; ///< Number of frames for each direction
    };
    /// @note offsets relative to the top-left screen pixel
    struct FrameHeader
    {
        int32_t  flip;      ///< Default (false) means scanlines are from bottom to top
        int32_t  width;     ///< Width in pixels
        int32_t  height;    ///< Height in pixels
        int32_t  offsetX;   ///< Horizontal offset for left edge of the frame
        int32_t  offsetY;   ///< Vertical offset for bottom(top if flipped) edge of the frame.
        int32_t  allocSize; ///< Used by the game as a slot to store the data size. 0 in the files.
        int32_t  nextBlock; ///< Offset/Pointer to the next frame
        uint32_t length;    ///< Length of the frame in chunks
    };

protected:
    StreamPtr                stream = nullptr;
    Header                   header;
    std::vector<uint32_t>    framePointers;
    std::vector<FrameHeader> frameHeaders;

    bool extractHeaders();

public:
    /**Start decoding the stream and preparing data.
     * @return true on success
     *
     * Prepares the decoder to read the frames using @ref decompressFrame.
     * Basically calls extractHeaders(), so that you can call getHeader() and getFrameHeaders().
     */
    bool initDecoder(StreamPtr&& streamPtr);

    /// Resets the decoder and frees resources
    void reset() { *this = DC6{}; }

    const Header&                   getHeader() const { return header; }
    const std::vector<FrameHeader>& getFrameHeaders() const { return frameHeaders; }

    /**Decompress the given frame.
     * @param frameNumber The frame number in the file
     * @return A valid vector to the data on success, empty one on failure
     */
    std::vector<uint8_t> decompressFrame(size_t frameNumber) const;

    /**Same as @ref decompressFrame but will output the data in a given buffer
     */
    bool decompressFrameIn(size_t frameNumber, uint8_t* data) const;

    void exportToPPM(const char* ppmFilenameBase, const Palette& palette) const;
};
} // namespace WorldStone
