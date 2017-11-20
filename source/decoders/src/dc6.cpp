//
// Created by Lectem on 23/10/2016.
//

#include "dc6.h"
#include <FileStream.h>
#include <fmt/format.h>
#include "palette.h"
#include "utils.h"

// TODO : Remove asserts and replace with proper error handling

namespace WorldStone
{
bool DC6::decode(const char* filename)
{
    assert(!stream);
    stream = std::make_unique<FileStream>(filename);
    if (stream && stream->good()) {
        return extractHeaders();
    }
    return false;
}

bool DC6::decode(StreamPtr&& streamPtr)
{
    assert(!stream);
    stream = std::move(streamPtr);
    if (stream && stream->good()) {
        return extractHeaders();
    }
    return false;
}

bool DC6::extractHeaders()
{
    static_assert(std::is_trivially_copyable<Header>(), "DC6::Header must be trivially copyable");
    static_assert(sizeof(Header) == 6 * sizeof(uint32_t), "DC6::Header struct needs to be packed");
    stream->read(&header, sizeof(header));
    if (stream->fail()) return false;

    size_t framesNumber = header.directions * header.framesPerDir;
    frameHeaders.resize(framesNumber);

    framePointers.resize(framesNumber);
    stream->read(framePointers.data(), sizeof(uint32_t) * framesNumber);
    if (stream->fail()) return false;

    for (size_t i = 0; i < framesNumber; ++i)
    {
        FrameHeader& frameHeader = frameHeaders[i];
        stream->seek(framePointers[i], IStream::beg);

        static_assert(std::is_trivially_copyable<FrameHeader>(),
                      "DC6::FrameHeader must be trivially copyable");
        static_assert(sizeof(FrameHeader) == 8 * sizeof(uint32_t),
                      "DC6::FrameHeader struct needs to be packed");
        stream->read(&frameHeader, sizeof(frameHeader));
        if (stream->fail()) return false;
    }
    return true;
}

std::vector<uint8_t> DC6::decompressFrame(size_t frameNumber) const
{
    const FrameHeader& fHeader = frameHeaders[frameNumber];
    // Allocate memory for the decoded data
    std::vector<uint8_t> data(static_cast<size_t>(fHeader.width * fHeader.height));

    decompressFrameIn(frameNumber, data.data());
    return data;
}

void DC6::decompressFrameIn(size_t frameNumber, uint8_t* data) const
{
    assert(stream != nullptr);
    stream->seek(framePointers[frameNumber] + sizeof(FrameHeader), IStream::beg);
    const FrameHeader& fHeader = frameHeaders[frameNumber];
    assert(fHeader.width > 0 && fHeader.height > 0);

    // TODO: figure if we should invert data here or let the renderer do it
    // assert(!fHeader.flip);

    // We're reading it bottom to top, but save data with the y axis from top to
    // bottom
    int x = 0, y = fHeader.height - 1;
    int rawIndex = 0;
    while (rawIndex < fHeader.length)
    {
        int val = stream->getc();
        rawIndex++;
        if (val == EOF) throw;
        uint8_t chunkSize = static_cast<uint8_t>(val);
        if (chunkSize == 0x80) // end of line
        {
            x = 0;
            y--;
        }
        else if (chunkSize & 0x80) // chunkSize & 0x80 is the number of transparent pixels
        {
            x += chunkSize & 0x7F;
        }
        else // chunkSize is the number of colors to read
        {
            assert(chunkSize + x <= fHeader.width);
            for (int i = 0; i < chunkSize; i++, x++)
            {
                int color = stream->getc();
                rawIndex++;
                if (color == EOF) throw;
                assert(x >= 0 && y >= 0 && x + fHeader.width * y < fHeader.width * fHeader.height);
                data[static_cast<size_t>(x + fHeader.width * y)] = static_cast<uint8_t>(color);
            }
        }
    }
    assert(fHeader.length == rawIndex);
}

void DC6::exportToPPM(const char* ppmFilenameBase, const Palette& palette) const
{
    for (uint32_t dir = 0; dir < header.directions; ++dir)
    {
        for (size_t frameInDir = 0; frameInDir < header.framesPerDir; ++frameInDir)
        {
            size_t frame = dir * header.framesPerDir + frameInDir;
            auto   data  = decompressFrame(frame);
            Utils::exportToPPM(fmt::format("{}{}-{}.ppm", ppmFilenameBase, dir, frameInDir).c_str(),
                               data.data(), frameHeaders[frame].width, frameHeaders[frame].height,
                               palette);
        }
    }
}
} // namespace WorldStone
