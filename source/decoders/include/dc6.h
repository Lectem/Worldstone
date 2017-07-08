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
 * @brief Decoder for the DC6 image format
 *
 * The DC6 (Diablo Cel 6) format is a compressed sprite image format.
 * It can store multiple directions and frames in a single file.
 * A palette is used to generate the colors, but the file itself doesn't have any information about
 * the palette to use, it is up to the user to know which one to use. @see WorldStone::Palette
 *
 * This format is mostly used for menu, items but also for some monsters (eg:Mephisto)
 * This is an update of diablo 1 Cel format
 */
class DC6
{
public:
    struct Header
    {
        int32_t  version;        ///< DC6 major version, usually 6
        int32_t  sub_version;    ///< DC6 minor version, usually 1
        int32_t  zeros;          ///< Always 0 ? might be micro version or encoding
        uint8_t  termination[4]; ///< Always 0xEEEEEEEE or 0xCDCDCDCD
        uint32_t directions;     ///< Number of directions in this file
        uint32_t frames_per_dir; ///< Number of frames for each direction
    };

    struct FrameHeader
    {
        int32_t flip;     ///< Default (false) means scanlines are from bottom to top
        int32_t width;    ///< Width in pixels
        int32_t height;   ///< Height in pixels
        int32_t offset_x; ///< Horizontal offset for left edge of the frame
        int32_t offset_y; ///< Vertical offset for bottom(top if flipped) edge ofthe
                          /// frame.
        int32_t zeros;
        int32_t next_block; ///< Pointer to the next frame
        int32_t length;     ///< Length of the frame in chunks
    };

protected:
    StreamPtr                stream = nullptr;
    Header                   header;
    std::vector<uint32_t>    framePointers;
    std::vector<FrameHeader> frameHeaders;

public:
    bool Decode(const char* filename);
    bool Decode(StreamPtr&& streamPtr);

    /// Resets the decoder and frees resources
    void Reset() { *this = DC6{}; }

    bool extractHeaders();

    const Header&                   getHeader() { return header; }
    const std::vector<FrameHeader>& getFameHeaders() { return frameHeaders; }

    /**
     * Decompress the given frame
     * @param frameNumber The frame number in the file
     * @return A valid vector to the data on success, empty one on failure
     */
    std::vector<uint8_t> decompressFrame(size_t frameNumber) const;
    /**
     * Same as decompressFrame but will output the data in a given buffer
     * @warning: asserts on failure
     */
    void decompressFrameIn(size_t frameNumber, uint8_t* data) const;

    void exportToPPM(const char* ppmFilenameBase, const Palette& palette) const;
};
} // namespace WorldStone
