#pragma once

#include <cstdio>
#include <stdint.h>
#include <FileStream.h>
#include <memory>
#include <type_traits>
#include <vector>
#include "palette.h"

namespace WorldStone
{
class DC6
{
public:
    struct Header
    {
        int32_t  version;        ///< DC6 major version, usually 6
        int32_t  sub_version;    ///< DC6 minor version, usually 1
        int32_t  zeros;          ///< Always 0 ? might be micro version or encoding
        uint8_t  pad_bytes[4];   ///< Always 0xEEEEEEEE or 0xCDCDCDCD
        uint32_t directions;     ///< Number of directions in this file
        uint32_t frames_per_dir; ///< Number of frames for each direction
    };
    static_assert(std::is_trivially_copyable<Header>(), "DC6::Header must be trivially copyable");
    static_assert(sizeof(Header) == 6 * sizeof(uint32_t), "DC6::Header struct needs to be packed");

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
        int32_t length;
    };
    static_assert(std::is_trivially_copyable<FrameHeader>(),
                  "DC6::FrameHeader must be trivially copyable");
    static_assert(sizeof(FrameHeader) == 8 * sizeof(uint32_t),
                  "DC6::FrameHeader struct needs to be packed");

protected:
    StreamPtr                stream = nullptr;
    Header                   header;
    std::vector<uint32_t>    framePointers;
    std::vector<FrameHeader> frameHeaders;

public:
    void Decode(const char* filename);
    void Decode(StreamPtr&& streamPtr);

    /// Resets the decoder and frees resources
    void Reset() { *this = DC6{}; }

    bool extractHeaders();

    const Header&                   getHeader() { return header; }
    const std::vector<FrameHeader>& getFameHeaders() { return frameHeaders; }

    /**
     * Decompress the given frame
     * @param frameNumber The frame number in the file
     * @return A valid pointer to the data on success, nullptr on failure
     */
    std::vector<uint8_t> decompressFrame(size_t frameNumber) const;

    void exportToPPM(const char* ppmFilenameBase, const Palette& palette) const;
};
}
