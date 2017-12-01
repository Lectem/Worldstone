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

using ReadUnsignedPtrType = uint32_t (WorldStone::BitStream::*)(void);
using readSignedPtrType   = int32_t (WorldStone::BitStream::*)(void);

static constexpr ReadUnsignedPtrType readUnsignedPtrs[16] = {
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

bool DCC::decode(const char* filename)
{
    assert(!stream);
    stream = std::make_unique<FileStream>(filename);
    if (stream && stream->good()) {
        return extractHeaderAndOffsets();
    }
    return false;
}

bool DCC::decode(StreamPtr&& streamPtr)
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
    stream->readRaw(header.framesPerDir);
    stream->readRaw(header.padding0);
    stream->readRaw(header.tag);            // TODO : ENDIAN
    stream->readRaw(header.finalDc6Size);   // TODO : ENDIAN

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

static bool readDirHeader(DCC::DirectionHeader& dirHeader, BitStream& bitStream)
{
    dirHeader.outsizeCoded          = bitStream.readUnsigned<32>();
    dirHeader.hasRawPixelEncoding   = bitStream.readBool();
    dirHeader.compressEqualCells    = bitStream.readBool();
    dirHeader.variable0Bits         = bitStream.readUnsigned<4>();
    dirHeader.widthBits             = bitStream.readUnsigned<4>();
    dirHeader.heightBits            = bitStream.readUnsigned<4>();
    dirHeader.xoffsetBits           = bitStream.readUnsigned<4>();
    dirHeader.yoffsetBits           = bitStream.readUnsigned<4>();
    dirHeader.optionalBytesBits     = bitStream.readUnsigned<4>();
    dirHeader.codedBytesBits        = bitStream.readUnsigned<4>();
    return bitStream.good();
}

static bool readFrameHeaders(uint8_t nbFrames, DCC::Direction& outDir, BitStream& bitStream)
{
    const DCC::DirectionHeader& dirHeader = outDir.header;

    // Read all frame headers
    outDir.frameHeaders.resize(nbFrames);
    for (DCC::FrameHeader& frameHeader : outDir.frameHeaders)
    {
        // We are using member function pointers here because we would have one indirection
        // From looking up the size anyway, so we might as well call the template instance directly
        frameHeader.variable0 = (bitStream.*readUnsignedPtrs[dirHeader.variable0Bits])();
        frameHeader.width     = (bitStream.*readUnsignedPtrs[dirHeader.widthBits])();
        frameHeader.height    = (bitStream.*readUnsignedPtrs[dirHeader.heightBits])();
        frameHeader.xoffset   = (bitStream.*readSignedPtrs[dirHeader.xoffsetBits])();
        frameHeader.yoffset   = (bitStream.*readSignedPtrs[dirHeader.yoffsetBits])();

        frameHeader.optionalBytes = (bitStream.*readUnsignedPtrs[dirHeader.optionalBytesBits])();
        frameHeader.codedBytes    = (bitStream.*readUnsignedPtrs[dirHeader.codedBytesBits])();
        frameHeader.frameBottomUp = bitStream.readBool();

        assert(frameHeader.width < 0x700000);
        assert(frameHeader.height < 0x700000);
        frameHeader.extents.xLower = frameHeader.xoffset;
        frameHeader.extents.xUpper = frameHeader.xoffset + int32_t(frameHeader.width);

        if (frameHeader.frameBottomUp) {
            frameHeader.extents.yLower = frameHeader.yoffset;
            frameHeader.extents.yUpper = frameHeader.yoffset + int32_t(frameHeader.height);
        }
        else // top-down
        {
            frameHeader.extents.yLower = frameHeader.yoffset - int32_t(frameHeader.height) + 1;
            frameHeader.extents.yUpper = frameHeader.yoffset + 1;
        }
    }

    // Handle optional data
    for (DCC::FrameHeader& frameHeader : outDir.frameHeaders)
    {
        if (frameHeader.optionalBytes) {
            bitStream.alignToByte();
            bitStream.skip(frameHeader.optionalBytes * CHAR_BIT);
        }
    }
    return bitStream.good();
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

    DirectionHeader& dirHeader = outDir.header;
    if (!readDirHeader(dirHeader, bitStream)) return false;

    if (!readFrameHeaders(header.framesPerDir, outDir, bitStream)) return false;

    outDir.computeDirExtents();

    uint32_t equalCellsBitStreamSize    = 0;
    uint32_t pixelMaskBitStreamSize     = 0;
    uint32_t encodingTypeBitsreamSize   = 0;
    uint32_t rawPixelCodesBitStreamSize = 0;

    if (dirHeader.compressEqualCells) {
        equalCellsBitStreamSize = bitStream.readUnsigned<20>();
    }

    pixelMaskBitStreamSize = bitStream.readUnsigned<20>();

    if (dirHeader.hasRawPixelEncoding) {
        encodingTypeBitsreamSize   = bitStream.readUnsigned<20>();
        rawPixelCodesBitStreamSize = bitStream.readUnsigned<20>();
    }
    // Tells what code correspond to which pixel value.
    // For example if the pixel values used are 0, 31 , 42 then
    // code 0 gives 0
    // code 1 gives 31
    // code 2 gives 42
    std::vector<uint8_t> codeToPixelValue;
    for (size_t i = 0; i < 256; i++)
    {
        // Not very efficient but will optimize later if needed
        const bool pixelValueUsed = bitStream.readBool();
        if (pixelValueUsed) codeToPixelValue.push_back(uint8_t(i));
    }

    BitStream equalCellBitStream;
    if (dirHeader.compressEqualCells) {
        equalCellBitStream = bitStream;
        bitStream.skip(equalCellsBitStreamSize);
    }

    BitStream pixelMaskBitStream = bitStream;
    bitStream.skip(pixelMaskBitStreamSize);

    BitStream rawPixelUsageBitStream;
    BitStream rawPixelCodesBitStream;
    if (dirHeader.hasRawPixelEncoding) {
        rawPixelUsageBitStream = bitStream;
        bitStream.skip(encodingTypeBitsreamSize);

        rawPixelCodesBitStream = bitStream;
        bitStream.skip(rawPixelCodesBitStreamSize);
    }

    // Note : goes until the end of the direction
    BitStream pixelCodesAndDisplacementBitStream = bitStream;

    struct Cell
    {
        size_t width : 4;
        size_t height : 4;
    };

    // Each pixel buffer entry contains 4 pixels codes
    struct PixelBufferEntry
    {
        enum
        {
            invalidIndex = -1
        };
        uint8_t values[4];
        int16_t frame          = -1;
        int16_t frameCellIndex = -1;
    };

    WS_UNUSED(equalCellBitStream);
    WS_UNUSED(pixelMaskBitStream);
    WS_UNUSED(rawPixelUsageBitStream);
    WS_UNUSED(rawPixelCodesBitStream);
    WS_UNUSED(pixelCodesAndDisplacementBitStream);

    const size_t dirWidth  = size_t(outDir.extents.width());
    const size_t dirHeight = size_t(outDir.extents.height());

    // Compute the size in cells of the pixel buffer. There are no alignment nor dimensions
    // requirements for the pixel buffer, but cells are of size 4 at max.
    const size_t cellMaxPixelSize = 4u;
    // nbPixelBufferCellsX = dirWidth/4 rounded up
    const size_t nbPixelBufferCellsX = 1u + (dirWidth - 1u) / cellMaxPixelSize;
    const size_t nbPixelBufferCellsY = 1u + (dirHeight - 1u) / cellMaxPixelSize;
    const size_t nbPixelBufferCells  = nbPixelBufferCellsX * nbPixelBufferCellsY;

    // Create the pixel buffer and its cells. For each cell we have a PixelBufferEntry pointer.
    constexpr size_t    invalidIndex = std::numeric_limits<size_t>::max();
    std::vector<size_t> pixelBuffer(nbPixelBufferCells, invalidIndex);

    std::vector<PixelBufferEntry> pixelBufferEntries;
    // TODO : put a Cell in the PixelBufferEntry instead ?
    std::vector<Cell> cells(nbPixelBufferCells, Cell{cellMaxPixelSize, cellMaxPixelSize});

    const size_t lastCellColumn = nbPixelBufferCellsX - 1u;
    const size_t lastCellRow    = nbPixelBufferCellsY - 1u;

    const size_t lastCellColumnWidth = dirWidth - lastCellColumn * cellMaxPixelSize;
    for (size_t y = 0; y < nbPixelBufferCellsY; y++)
    {
        Cell& cell = cells[lastCellColumn + y * nbPixelBufferCellsX];
        cell.width = lastCellColumnWidth;
    }
    const size_t lastCellRowHeight = dirHeight - lastCellRow * cellMaxPixelSize;
    for (size_t x = 0; x < nbPixelBufferCellsX; x++)
    {
        Cell& cell  = cells[x + lastCellRow * nbPixelBufferCellsX];
        cell.height = lastCellRowHeight;
    }

    struct FrameData
    {
        size_t            firstPixelBufferEntry;
        uint16_t          nbCellsX = 0;
        uint16_t          nbCellsY = 0;
        std::vector<bool> cellSameAsPrevious;
    };
    std::vector<FrameData> framesData(header.framesPerDir);

    // 1st phase of decoding : fill the pixel buffer
    for (size_t frameIndex = 0; frameIndex < header.framesPerDir; ++frameIndex)
    {
        const FrameHeader& frameHeader = outDir.frameHeaders[frameIndex];
        FrameData&         frameData    = framesData[frameIndex];
        frameData.firstPixelBufferEntry = pixelBufferEntries.size();

        // Create the cells for this frame
        constexpr size_t TOBECOMPUTED  = 1;
        const size_t     nbFrameCellsX = TOBECOMPUTED;
        const size_t     nbFrameCellsY = TOBECOMPUTED;
        frameData.cellSameAsPrevious.resize(nbFrameCellsX * nbFrameCellsY);

        const size_t frameOffsetX     = size_t(frameHeader.extents.xLower - outDir.extents.xLower);
        const size_t frameOffsetY     = size_t(frameHeader.extents.yLower - outDir.extents.yLower);
        const size_t frameCellOffsetX = frameOffsetX / 4;
        const size_t frameCellOffsetY = frameOffsetY / 4;

        // For each cell of this frame (not the same number as the pixel buffer cells ! )
        for (size_t y = 0; y < nbFrameCellsY; y++)
        {
            const size_t curCellY = frameCellOffsetY + y;
            for (size_t x = 0; x < nbFrameCellsX; x++)
            {
                const size_t curCellX                = frameCellOffsetX + x;
                const size_t curPixelBufferCellIndex = curCellX + curCellY * nbPixelBufferCellsX;
                const size_t curFrameCellIndex       = x + y * nbFrameCellsX;
                WS_UNUSED(curFrameCellIndex);
                // Note : Seems that you actually need one for every frame
                // should be indexed on curFrameCellIndex
                // In dccinfo cell_buffer is a ptr to the latest value for each cell of the global
                // framebuffer This saves us from going back from frame to frame to get the last
                // value of the cell (frames might not overlap) Eache frame pixelbuffer is then
                // reused in phase 2 for decoding It does NOT need have size nbFrameCells because of
                // the equal cells compression
                size_t& lastPixelEntryForCellIndex = pixelBuffer[curPixelBufferCellIndex];

                bool    sameAsPreviousCell = false; // By default always decode the cell
                uint8_t pixelMask          = 0x0F;  // Default pixel mask

                // Check if this cell is equal to the previous one
                if (lastPixelEntryForCellIndex < pixelBufferEntries.size()) {
                    // Check if we have to reuse the previous values
                    if (dirHeader.compressEqualCells) {
                        // If true, it means the cell is the same as the previous one, or
                        // transparent Which mean the same thing : skip the decoding of this cell
                        sameAsPreviousCell = equalCellBitStream.readBool();
                    }
                    if (!sameAsPreviousCell) {
                        pixelMask = pixelMaskBitStream.readUnsigned<4, uint8_t>();
                    }
                }
                // Store the fact that we skipped this cell for the 2nd phase
                frameData.cellSameAsPrevious[curFrameCellIndex] = sameAsPreviousCell;
                if (!sameAsPreviousCell) {
                    // Decode this cell pixel codes

                    const uint16_t nbPixelsInMask = Utils::popCount(uint16_t(pixelMask));

                    bool decodeRaw = rawPixelUsageBitStream.sizeInBits() > 0 &&
                                     rawPixelUsageBitStream.readBool();

                    // We need to decode this cell
                    uint8_t cellPixelCodesStack[4] = {0, 0, 0, 0};
                    uint8_t lastPixelCode          = 0;
                    size_t  nbPixelsDecoded        = 0;
                    for (uint16_t curPixelIdx = 0; curPixelIdx < nbPixelsInMask; curPixelIdx++)
                    {
                        uint8_t& curPixelCode = cellPixelCodesStack[curPixelIdx];
                        if (decodeRaw) {
                            // Read the value of the code directly from rawPixelCodesBitStream
                            curPixelCode = rawPixelCodesBitStream.readUnsigned<8, uint8_t>();
                        }
                        else
                        {
                            // Read the value of the code incrementally from
                            // pixelCodesAndDisplacementBitStream
                            curPixelCode = lastPixelCode;
                            uint8_t pixelDisplacement;
                            do
                            {
                                pixelDisplacement =
                                    pixelCodesAndDisplacementBitStream.readUnsigned<4, uint8_t>();
                                curPixelCode += pixelDisplacement;
                            } while (pixelDisplacement == 0xF);
                        }
                        // Stop decoding if we encounter twice the same pixel code.
                        // It also means that this pixel value is discarded.
                        if (curPixelCode == lastPixelCode) {
                            // We should be discarding it here, but it is easier in stage 2 if we
                            // don't. dccinfo might be broken as it decoded too many bits from
                            // pixelCodesAndDisplacementBitStream if only 1 pixel was decoded ? it
                            // checked if pixelValue[0] == pixelValue[1] but that never happened
                            // unless pixelCode[0] = 0 For example : if we decode from the stream c
                            // = { x , x , 0 , 0}, it was storing only codes c = { x, 0, 0, 0}...
                            // Then it thought it had to decode 1 bit for indexing since c[0] !=
                            // c[1] but c[1] == c[2], ending up choosing between x and 0 instead of
                            // just x and also changing all the following cells decoded values This
                            // is actually surprising since the results looked correct, it could be
                            // an issue in the DCC documentation though.
                            // curPixelCode = 0;
                            assert(curPixelIdx > 1 && "TODO : check that this is indeed wrong!");
                            break;
                        }
                        else
                        {
                            lastPixelCode = curPixelCode;
                            nbPixelsDecoded++;
                        }
                    }
                    // Can't hold pointer since pixelBufferEntries will grow, might not be worth the
                    // cost ?
                    const size_t     previousEntryForCellIndex = lastPixelEntryForCellIndex;
                    PixelBufferEntry previousEntryForCell;
                    if (previousEntryForCellIndex < pixelBufferEntries.size()) {
                        previousEntryForCell = pixelBufferEntries[previousEntryForCellIndex];
                    }
                    else
                    {
                        // We will not need previousEntryForCell if it doesn't exist,
                        // as the mask is 0xF
                        assert(pixelMask == 0xF);
                    }

                    // Might be possible to fill the entry directly instead of using this stack ?
                    PixelBufferEntry newEntry;
                    int curIndex = int(nbPixelsDecoded); // using int because we decrement until -1
                    for (size_t i = 0; i < 4; i++)
                    {
                        // Pop a value if bit set in the mask
                        if (pixelMask & (1u << i)) {
                            uint8_t pixelCode;
                            if (curIndex >= 0) {
                                pixelCode = cellPixelCodesStack[curIndex--];
                            }
                            else
                                pixelCode = 0;
                            // Store the actual values instead of the codes
                            newEntry.values[i] = codeToPixelValue[pixelCode];
                        }
                        // If not set, use the previous value for this entry
                        else
                        {
                            newEntry.values[i] = previousEntryForCell.values[i];
                        }
                    }
                    lastPixelEntryForCellIndex = pixelBufferEntries.size();
                    pixelBufferEntries.push_back(newEntry);
                }
            }
        }
    }

    // 2nd phase of decoding : Finish using the pixel buffer entries

    // 1st phase of decoding : fill the pixel buffer
    for (size_t frameIndex = 0; frameIndex < header.framesPerDir; ++frameIndex)
    {
        const FrameHeader& frameHeader = outDir.frameHeaders[frameIndex];
        WS_UNUSED(frameHeader);
        const FrameData& frameData = framesData[frameIndex];

        size_t nbFrameCells = frameData.nbCellsX * frameData.nbCellsX;
        assert(frameData.firstPixelBufferEntry + nbFrameCells <= pixelBufferEntries.size());
        for (size_t cellIndex = 0; cellIndex < nbFrameCells; cellIndex++)
        {
            Cell cell;
            if (frameData.cellSameAsPrevious[cellIndex]) {
                if (0 /*different size from previous cell ?*/) {
                    // All to 0
                }
                else
                {
                    // Same size, copy from previous cell in the buffer
                }
            }
            else
            {
                PixelBufferEntry entry =
                    pixelBufferEntries[frameData.firstPixelBufferEntry + cellIndex];
                const auto& pixelValues = entry.values;

                ReadUnsignedPtrType readCodeIndex = nullptr;
                if (pixelValues[0] == pixelValues[1]) {
                    // This means we only got one pixel code, so fill
                }
                else
                {
                    if (pixelValues[0] == pixelValues[1]) {
                        // Stopped decoding after the 2nd value,
                        // only 1bit needs to be read to choose from the 1st and 2nd values
                        readCodeIndex = &BitStream::readUnsigned<1>;
                    }
                    else
                        readCodeIndex = &BitStream::readUnsigned<2>;
                    ;

                    // fill FRAME cell with pixels
                    for (int y = 0; y < cell.height; y++)
                    {
                        for (int x = 0; x < cell.width; x++)
                        {
                            uint32_t pixelCodeIndex =
                                (pixelCodesAndDisplacementBitStream.*readCodeIndex)();
                            // Lectem's note:This actually means that a cell (4x4 block) can use at
                            // most 4 colors !
                            const uint8_t pixelValue = entry.values[pixelCodeIndex];
                            WS_UNUSED(pixelValue);
                        }
                    }
                }
            }
        }
    }

    return bitStream.good();
}
} // namespace WorldStone
