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

// constexpr unsigned DCC::bitsWidthTable[16] = {0,  1,  2,  4,  6,  8,  10, 12,
//                                              14, 16, 20, 24, 26, 28, 30, 32};

using readUnsignedPtrType = uint32_t (WorldStone::BitStream::*)(void);
using readSignedPtrType   = int32_t (WorldStone::BitStream::*)(void);

static constexpr readUnsignedPtrType readUnsignedPtrs[16] = {
    &BitStream::read0Bits,
    &BitStream::readBit,
    &BitStream::readUnsigned<DCC::bitsWidthTable[2]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[3]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[4]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[5]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[6]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[7]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[8]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[9]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[10]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[11]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[12]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[13]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[14]>,
    &BitStream::readUnsigned<DCC::bitsWidthTable[15]>};

static constexpr readSignedPtrType readSignedPtrs[16] = {
    &BitStream::readSigned<DCC::bitsWidthTable[0]>,
    &BitStream::readSigned<DCC::bitsWidthTable[1]>,
    &BitStream::readSigned<DCC::bitsWidthTable[2]>,
    &BitStream::readSigned<DCC::bitsWidthTable[3]>,
    &BitStream::readSigned<DCC::bitsWidthTable[4]>,
    &BitStream::readSigned<DCC::bitsWidthTable[5]>,
    &BitStream::readSigned<DCC::bitsWidthTable[6]>,
    &BitStream::readSigned<DCC::bitsWidthTable[7]>,
    &BitStream::readSigned<DCC::bitsWidthTable[8]>,
    &BitStream::readSigned<DCC::bitsWidthTable[9]>,
    &BitStream::readSigned<DCC::bitsWidthTable[10]>,
    &BitStream::readSigned<DCC::bitsWidthTable[11]>,
    &BitStream::readSigned<DCC::bitsWidthTable[12]>,
    &BitStream::readSigned<DCC::bitsWidthTable[13]>,
    &BitStream::readSigned<DCC::bitsWidthTable[14]>,
    &BitStream::readSigned<DCC::bitsWidthTable[15]>};

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
    BitStream bitStream(buffer.data(), directionEncodedSize);

    // Read the direction header
    DirectionHeader& dirHeader      = outDir.header;
    dirHeader.outsize_coded         = bitStream.readUnsigned<32>();
    dirHeader.compressColorEncoding = bitStream.readBool();
    dirHeader.compressEqualCells    = bitStream.readBool();
    dirHeader.variable0_bits        = bitStream.readUnsigned<4>();
    dirHeader.width_bits            = bitStream.readUnsigned<4>();
    dirHeader.height_bits           = bitStream.readUnsigned<4>();
    dirHeader.xoffset_bits          = bitStream.readUnsigned<4>();
    dirHeader.yoffset_bits          = bitStream.readUnsigned<4>();
    dirHeader.optional_bytes_bits   = bitStream.readUnsigned<4>();
    dirHeader.coded_bytes_bits      = bitStream.readUnsigned<4>();

    // Read all frame headers
    outDir.frameHeaders.resize(header.frames_per_dir);
    for (FrameHeader& frameHeader : outDir.frameHeaders)
    {
        // We are using member function pointers here because we would have one indirection
        // From looking up the size anyway, so we might as well call the template instance directly
        frameHeader.variable0 = (bitStream.*readUnsignedPtrs[dirHeader.variable0_bits])();
        frameHeader.width     = (bitStream.*readUnsignedPtrs[dirHeader.width_bits])();
        frameHeader.height    = (bitStream.*readUnsignedPtrs[dirHeader.height_bits])();
        frameHeader.xoffset   = (bitStream.*readSignedPtrs[dirHeader.xoffset_bits])();
        frameHeader.yoffset   = (bitStream.*readSignedPtrs[dirHeader.yoffset_bits])();

        frameHeader.optional_bytes = (bitStream.*readUnsignedPtrs[dirHeader.optional_bytes_bits])();
        frameHeader.coded_bytes    = (bitStream.*readUnsignedPtrs[dirHeader.coded_bytes_bits])();
        frameHeader.frame_bottom_up = bitStream.readBool();
    }

    // Handle optional data
    for (FrameHeader& frameHeader : outDir.frameHeaders)
    {
        if (frameHeader.optional_bytes) {
            bitStream.alignToByte();
            bitStream.skip(frameHeader.optional_bytes * CHAR_BIT);
        }
    }

    // TODO : Read the frame content
    return bitStream.good();
}
} // namespace WorldStone
