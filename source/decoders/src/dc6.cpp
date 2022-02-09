//
// Created by Lectem on 23/10/2016.
//

#include "dc6.h"
#include <FileStream.h>
#include <SystemUtils.h>
#include <fmt/format.h>
#include "Palette.h"
#include "utils.h"

// TODO : Remove asserts and replace with proper error handling

namespace WorldStone
{

bool DC6::initDecoder(StreamPtr&& streamPtr)
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

    size_t framesNumber = size_t(header.directions) * size_t(header.framesPerDir);
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

    if (decompressFrameIn(frameNumber, data.data()))return data;
    else return {};
}

bool DC6::decompressFrameIn(size_t frameNumber, uint8_t* data) const
{
    assert(stream != nullptr);
    stream->seek(framePointers[frameNumber] + sizeof(FrameHeader), IStream::beg);
    const FrameHeader& fHeader = frameHeaders[frameNumber];
    assert(fHeader.width > 0 && fHeader.height > 0);
    
    // Eat any leading 0s. Blizzard somehow changed and fucked up the encoding or serialization in D2:Remaster
    // The 3 additional trailing bytes that used to be garbage at the end of the data are now replaced with leading 0s
    // Those are NOT counted by the FrameHeader::length member, so ignore them
    int leadingZeros = 0;
    int val = -1;
    do {
        val = stream->getc();
        if(val == 0)
            leadingZeros++;
    } while (val == 0);
    // These are the only values we encountered so far, so report if you find another.
    assert(leadingZeros == 0 || leadingZeros == 3);


    // TODO: figure if we should invert data here or let the renderer do it
    // assert(!fHeader.flip);

    // We're reading it bottom to top, but save data with the y axis from top to
    // bottom
    int    x = 0, y = fHeader.height - 1;
    size_t rawIndex;
    for (rawIndex = 0; rawIndex < fHeader.length; val = stream->getc())
    {
        if (stream->eof() || val == -1) return false;

        rawIndex++; // We ate a character from the stream

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
            stream->read(data + x + fHeader.width * y, chunkSize);
            rawIndex += chunkSize;
            x += chunkSize;
            if (stream->eof()) return false;
        }
    }
    assert(fHeader.length == rawIndex);
    return true;
}

void DC6::exportToPPM(const char* ppmFilenameBase, const Palette& palette) const
{
    for (size_t dir = 0; dir < header.directions; ++dir)
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
