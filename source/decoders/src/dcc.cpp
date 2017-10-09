//
// Created by Lectem.
//

#include "dcc.h"
#include <BitStream.h>
#include <assert.h>
#include "palette.h"
#include "utils.h"

namespace WorldStone
{

const int DCC::bitsWidthTable[16] = {0, 1, 2, 4, 6, 8, 10, 12, 14, 16, 20, 24, 26, 28, 30, 32};

bool DCC::Decode(const char* filename)
{
    assert(!stream);
    stream = std::make_unique<FileStream>(filename);
    if (stream && stream->good()) {
        return extractHeaderAndOffsets();
    }
    return false;
}

bool DCC::Decode(StreamPtr&& streamPtr)
{
    assert(!stream);
    stream = std::move(streamPtr);
    if (stream && stream->good()) {
        return extractHeaderAndOffsets();
    }
    return false;
}

bool DCC::extractHeaderAndOffsets()
{
    // For now assume the stream is the whole file, and starts at offset 0
    assert(stream->tell() == 0);
    // DCC header can not encode a bigger size anyway
    assert(stream->size() < std::numeric_limits<int32_t>::max());
    stream->readRaw(header.signature);
    stream->readRaw(header.version);
    stream->readRaw(header.directions);
    stream->readRaw(header.frames_per_dir);
    stream->readRaw(header.padding0);
    stream->readRaw(header.tag);            // TODO : ENDIAN
    stream->readRaw(header.final_dc6_size); // TODO : ENDIAN

    assert(header.padding0[0] == 0 && header.padding0[1] == 0 && header.padding0[2] == 0 &&
           "Assumed there are 255 frames max, but Paul Siramy's doc mentions 256 as max ?");

    directionsOffsets.resize(header.directions + 1);
    directionsOffsets[header.directions] = static_cast<uint32_t>(stream->size());
    for (uint32_t dir = 0; dir < header.directions; dir++)
    {
        stream->readRaw(directionsOffsets[dir]); // TODO : ENDIAN
    }
    return stream->good();
}

size_t DCC::getDirectionSize(uint32_t dirIndex)
{
    return directionsOffsets[dirIndex + 1] - directionsOffsets[dirIndex];
}

bool DCC::readDirection(Direction& outDir, uint32_t dirIndex)
{
    using byte                             = unsigned char*;
    const size_t      directionEncodedSize = getDirectionSize(dirIndex);
    std::vector<byte> buffer(directionEncodedSize);
    stream->seek(directionsOffsets[dirIndex], IStream::beg);
    stream->read(buffer.data(), directionEncodedSize);
    assert(stream->good());
    BitStream        dirBitStream(buffer.data(), directionEncodedSize);
    DirectionHeader& dirHeader      = outDir.header;
    dirHeader.outsize_coded         = dirBitStream.readUnsigned<32>();
    dirHeader.compressColorEncoding = dirBitStream.readUnsigned<1>() != 0;
    dirHeader.compressEqualCells    = dirBitStream.readUnsigned<1>() != 0;
    dirHeader.variable0_bits        = dirBitStream.readUnsigned<4>();
    dirHeader.width_bits            = dirBitStream.readUnsigned<4>();
    dirHeader.height_bits           = dirBitStream.readUnsigned<4>();
    dirHeader.xoffset_bits          = dirBitStream.readUnsigned<4>();
    dirHeader.yoffset_bits          = dirBitStream.readUnsigned<4>();
    dirHeader.optional_bytes_bits   = dirBitStream.readUnsigned<4>();
    dirHeader.coded_bytes_bits      = dirBitStream.readUnsigned<4>();

    return dirBitStream.good();
}
} // namespace WorldStone
