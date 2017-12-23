/// @internal
/** @file dcc.cpp
 * @author Lectem.
 * Thanks to Bilian Belchev and Paul Siramy for their DCC file format documentation, upon which this
 * decoder is hugely based
 */

#include "dcc.h"
#include <BitStream.h>
#include <array>
#include <assert.h>
#include <fmt/format.h>
#include "ImageView.h"
#include "palette.h"
#include "utils.h"

namespace WorldStone
{

constexpr unsigned DCC::bitsWidthTable[16];
// constexpr unsigned DCC::bitsWidthTable[16] = {0,  1,  2,  4,  6,  8,  10, 12,
//                                              14, 16, 20, 24, 26, 28, 30, 32};

using readSignedPtrType   = int32_t (WorldStone::BitStreamView::*)(void);

static constexpr readSignedPtrType readSignedPtrs[16] = {
    &BitStreamView::readSigned<DCC::bitsWidthTable[0]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[1]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[2]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[3]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[4]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[5]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[6]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[7]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[8]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[9]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[10]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[11]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[12]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[13]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[14]>,
    &BitStreamView::readSigned<DCC::bitsWidthTable[15]>};

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
    directionsOffsets[header.directions] = uint32_t(stream->size());
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

static bool readDirHeader(DCC::DirectionHeader& dirHeader, BitStreamView& bitStream)
{
    dirHeader.outsizeCoded          = bitStream.readUnsigned(32);
    dirHeader.hasRawPixelEncoding   = bitStream.readBool();
    dirHeader.compressEqualCells    = bitStream.readBool();
    dirHeader.variable0Bits         = bitStream.readUnsigned8OrLess(4);
    dirHeader.widthBits             = bitStream.readUnsigned8OrLess(4);
    dirHeader.heightBits            = bitStream.readUnsigned8OrLess(4);
    dirHeader.xOffsetBits           = bitStream.readUnsigned8OrLess(4);
    dirHeader.yOffsetBits           = bitStream.readUnsigned8OrLess(4);
    dirHeader.optionalBytesBits     = bitStream.readUnsigned8OrLess(4);
    dirHeader.codedBytesBits        = bitStream.readUnsigned8OrLess(4);
    return bitStream.good();
}

static bool readFrameHeaders(uint8_t nbFrames, DCC::Direction& outDir, BitStreamView& bitStream)
{
    constexpr auto              bitsWidthTable = DCC::bitsWidthTable;
    const DCC::DirectionHeader& dirHeader = outDir.header;
    // Read all frame headers
    outDir.frameHeaders.resize(nbFrames);
    for (DCC::FrameHeader& fHdr : outDir.frameHeaders)
    {
        // We are using member function pointers here because we would have one indirection
        // From looking up the size anyway, so we might as well call the template instance directly
        fHdr.variable0 = bitStream.readUnsigned(bitsWidthTable[dirHeader.variable0Bits]);
        fHdr.width     = bitStream.readUnsigned(bitsWidthTable[dirHeader.widthBits]);
        fHdr.height    = bitStream.readUnsigned(bitsWidthTable[dirHeader.heightBits]);
        fHdr.xOffset   = (bitStream.*readSignedPtrs[dirHeader.xOffsetBits])();
        fHdr.yOffset   = (bitStream.*readSignedPtrs[dirHeader.yOffsetBits])();

        fHdr.optionalBytes = bitStream.readUnsigned(bitsWidthTable[dirHeader.optionalBytesBits]);
        fHdr.codedBytes    = bitStream.readUnsigned(bitsWidthTable[dirHeader.codedBytesBits]);
        fHdr.frameBottomUp = bitStream.readBool();

        assert(fHdr.width < 0x700000);
        assert(fHdr.height < 0x700000);
        fHdr.extents.xLower = fHdr.xOffset;
        fHdr.extents.xUpper = fHdr.xOffset + int32_t(fHdr.width);

        if (fHdr.frameBottomUp) {
            assert(false && "Please report the name of the DCC file to the devs!");
            fHdr.extents.yLower = fHdr.yOffset;
            fHdr.extents.yUpper = fHdr.yOffset + int32_t(fHdr.height);
        }
        else // top-down
        {
            fHdr.extents.yLower = fHdr.yOffset - int32_t(fHdr.height) + 1;
            fHdr.extents.yUpper = fHdr.yOffset + 1;
        }
    }

    // Handle optional data
    for (DCC::FrameHeader& frameHeader : outDir.frameHeaders)
    {
        if (frameHeader.optionalBytes) {
            assert(false && "Please report the name of the DCC file to the devs!");
            bitStream.alignToByte();
            bitStream.skip(frameHeader.optionalBytes * CHAR_BIT);
        }
    }
    return bitStream.good();
}
namespace
{ // Do not expose internals
// For the pixel buffer the maximum size of a cell is 4
constexpr size_t pbCellMaxPixelSize = 4u;

struct Cell
{
    size_t width : 4;  // 3 bits would be engouh (max value is 5)
    size_t height : 4; // 3 bits would be engouh (max value is 5)
};

// Each pixel buffer entry contains 4 pixels codes
struct PixelBufferEntry
{
    static constexpr size_t nbValues = 4;
    uint8_t                 values[nbValues];
};

struct FrameData
{
    using CellSize = uint8_t;

    size_t   firstPixelBufferEntry;
    uint16_t nbCellsX;
    uint16_t nbCellsY;
    uint16_t offsetX; ///< X Offset relative to the whole direction bounding box
    uint16_t offsetY; ///< Y Offset relative to the whole direction bounding box

    Vector<bool>     cellSameAsPrevious;
    Vector<CellSize> cellWidths;
    Vector<CellSize> cellHeights;

    ImageView<uint8_t> imageView; ///< Output buffer image view
    FrameData(const DCC::Direction& dir, const DCC::FrameHeader& frameHeader,
              IImageProvider<uint8_t>& imgProvider)
    {
        offsetX = uint16_t(frameHeader.extents.xLower - dir.extents.xLower);
        offsetY = uint16_t(frameHeader.extents.yLower - dir.extents.yLower);

        // width (in # of pixels) in 1st column
        const uint16_t widthFirstColumn = 4 - (offsetX % 4);
        const uint16_t frameWidth       = uint16_t(frameHeader.extents.width());
        if ((frameWidth - widthFirstColumn) <= 1)
            nbCellsX = 1; // if 2nd column is 0 or 1 pixel wide, only use 1 cell
        else
        {
            // so, we have minimum 2 pixels behind 1st column
            uint16_t tmp = frameWidth - widthFirstColumn - 1; // tmp is minimum 1, can't be 0
            nbCellsX     = 2 + (tmp / 4);
            if ((tmp % 4) == 0) nbCellsX--;
        }

        const uint16_t heightFirstRow = 4 - (offsetY % 4);
        const uint16_t frameHeight    = uint16_t(frameHeader.extents.height());
        if ((frameHeight - heightFirstRow) <= 1)
            nbCellsY = 1; // if 2nd row is 0 or 1 pixel high, only use 1 cell
        else
        {
            uint16_t tmp = frameHeight - heightFirstRow - 1;
            nbCellsY     = 2 + (tmp / 4);
            if ((tmp % 4) == 0) nbCellsY--;
        }

        cellSameAsPrevious.resize(nbCellsX * nbCellsY);

        // Initialize to 4 by default
        cellWidths.resize(nbCellsX, 4);
        cellHeights.resize(nbCellsY, 4);

        if (nbCellsX == 1)
            cellWidths[0] = CellSize(frameWidth); // Might have merged 2nd column into 1st
        else
        {
            // Treat the special cases (first and last columns/rows)
            cellWidths[0] = CellSize(widthFirstColumn);
            // Compute size of the last column
            const size_t nbColumnsExcludingFirstAndLast    = nbCellsX - 2;
            const size_t widthExcludingFirstAndLastColumns = 4 * nbColumnsExcludingFirstAndLast;
            cellWidths[nbCellsX - 1] =
                CellSize(frameWidth - (widthFirstColumn + widthExcludingFirstAndLastColumns));
        }

        if (nbCellsY == 1)
            cellHeights[0] = CellSize(frameHeight); // Might have merged 2nd row into 1st
        else
        {
            cellHeights[0] = CellSize(heightFirstRow);
            // Compute size of the last row
            const size_t nbRowsExcludingFirstAndLast     = nbCellsY - 2;
            const size_t heightExcludingFirstAndLastRows = 4 * nbRowsExcludingFirstAndLast;
            cellHeights[nbCellsY - 1] =
                CellSize(frameHeight - (heightFirstRow + heightExcludingFirstAndLastRows));
        }

        imageView = imgProvider.getNewImage(frameWidth, frameHeight);
    }
};

struct DirectionData
{
    const DCC::Direction& dirRef;

    Vector<uint8_t> codeToPixelValue;

    BitStreamView equalCellBitStream;
    BitStreamView pixelMaskBitStream;
    BitStreamView rawPixelUsageBitStream;
    BitStreamView rawPixelCodesBitStream;
    BitStreamView pixelCodesDisplacementBitStream;

    size_t nbFrames;
    size_t nbPixelBufferCellsX;
    size_t nbPixelBufferCellsY;

    Vector<FrameData> framesData;

    DirectionData(const DCC::Direction& dir, BitStreamView& bitStream, size_t nbFramesPerDir,
                  IImageProvider<uint8_t>& imgProvider)
        : dirRef(dir), nbFrames(nbFramesPerDir)
    {
        uint32_t equalCellsBitStreamSize    = 0;
        uint32_t pixelMaskBitStreamSize     = 0;
        uint32_t encodingTypeBitsreamSize   = 0;
        uint32_t rawPixelCodesBitStreamSize = 0;

        if (dir.header.compressEqualCells) {
            equalCellsBitStreamSize = bitStream.readUnsigned(20);
        }

        pixelMaskBitStreamSize = bitStream.readUnsigned(20);

        if (dir.header.hasRawPixelEncoding) {
            encodingTypeBitsreamSize   = bitStream.readUnsigned(20);
            rawPixelCodesBitStreamSize = bitStream.readUnsigned(20);
        }

        // Tells what code correspond to which pixel value.
        // For example if the pixel values used are 0, 31 , 42 then
        // code 0 gives 0
        // code 1 gives 31
        // code 2 gives 42
        codeToPixelValue.reserve(256);
        for (size_t i = 0; i < 256; i++)
        {
            const bool pixelValueUsed = bitStream.readBool();
            if (pixelValueUsed) codeToPixelValue.push_back(uint8_t(i));
        }

        // Prepare the bitstreams

        assert(!dir.header.compressEqualCells || equalCellsBitStreamSize);
        equalCellBitStream = bitStream.createSubView(equalCellsBitStreamSize);
        bitStream.skip(equalCellsBitStreamSize);

        pixelMaskBitStream = bitStream.createSubView(pixelMaskBitStreamSize);
        bitStream.skip(pixelMaskBitStreamSize);

        assert(!dir.header.hasRawPixelEncoding ||
               (encodingTypeBitsreamSize && rawPixelCodesBitStreamSize));
        rawPixelUsageBitStream = bitStream.createSubView(encodingTypeBitsreamSize);
        bitStream.skip(encodingTypeBitsreamSize);

        rawPixelCodesBitStream = bitStream.createSubView(rawPixelCodesBitStreamSize);
        bitStream.skip(rawPixelCodesBitStreamSize);

        // Note : goes until the end of the direction
        pixelCodesDisplacementBitStream =
            bitStream.createSubView(bitStream.bufferSizeInBits() - bitStream.tell());

        const size_t dirWidth  = size_t(dir.extents.width());
        const size_t dirHeight = size_t(dir.extents.height());

        // Compute the size in cells of the pixel buffer. There are no alignment nor dimensions
        // requirements for the pixel buffer, but cells are of size 4 at max.

        // nbPixelBufferCellsX = dirWidth/4 rounded up
        nbPixelBufferCellsX = 1u + (dirWidth - 1u) / pbCellMaxPixelSize;
        nbPixelBufferCellsY = 1u + (dirHeight - 1u) / pbCellMaxPixelSize;

        framesData.reserve(nbFrames);

        for (size_t frameIndex = 0; frameIndex < nbFrames; ++frameIndex)
        {
            framesData.emplace_back(dir, dir.frameHeaders[frameIndex], imgProvider);
        }
    }

    /// Check that all the data is valid and allocated
    bool isValid() const
    {
        for (size_t frameIndex = 0; frameIndex < nbFrames; ++frameIndex)
        {
            if (!framesData[frameIndex].imageView.isValid()) return false;
        }
        return true;
    }
};

using PixelCodesStack = std::array<uint8_t, PixelBufferEntry::nbValues>;
/**
 * @return the number of pixels codes decoded from the stream
 */
int decodePixelCodesStack(DirectionData& data, uint8_t pixelMask, PixelCodesStack& pixelCodesStack)
{
    if (!pixelMask) return 0; // Reuse the previous cell values, but still decode the cell in stage2
    const uint16_t nbPixelsInMask = Utils::popCount(uint16_t(pixelMask));

    // Is the cell encoded in the raw stream ?
    const bool decodeRaw = data.rawPixelUsageBitStream.bufferSizeInBits() > 0 &&
                           data.rawPixelUsageBitStream.readBool();

    uint8_t lastPixelCode = 0;
    size_t  curPixelIdx   = 0;

    for (curPixelIdx = 0; curPixelIdx < nbPixelsInMask; curPixelIdx++)
    {
        uint8_t& curPixelCode = pixelCodesStack[curPixelIdx];
        if (decodeRaw) {
            // Read the value of the code directly from rawPixelCodesBitStream
            curPixelCode = data.rawPixelCodesBitStream.readUnsigned8OrLess(8);
        }
        else
        {
            // Read the value of the code incrementally from pixelCodesDisplacementBitStream
            curPixelCode = lastPixelCode;
            uint8_t pixelDisplacement;
            do
            {
                pixelDisplacement = data.pixelCodesDisplacementBitStream.readUnsigned8OrLess(4);
                curPixelCode += pixelDisplacement;
            } while (pixelDisplacement == 0xF);
        }
        // Stop decoding if we encounter twice the same pixel code.
        // It also means that this pixel code is discarded.
        if (curPixelCode == lastPixelCode) {
            // Note : We discard the pixel by putting a 0 but it doesn't matter anyway since we only
            // use nbPixelsDecoded values later when popping the stack
            curPixelCode = 0;
            break;
        }
        else
        {
            lastPixelCode = curPixelCode;
        }
    }
    return int(curPixelIdx);
}

void decodeFrameStage1(DirectionData& data, FrameData& frameData, Vector<size_t>& pixelBuffer,
                       Vector<PixelBufferEntry>& pbEntries)
{
    // Offset in terms of cells for this frame
    const size_t frameCellOffsetX = frameData.offsetX / 4;
    const size_t frameCellOffsetY = frameData.offsetY / 4;

    // For each cell of this frame (not the same number as the pixel buffer cells ! )
    for (size_t y = 0; y < frameData.nbCellsY; y++)
    {
        const size_t curCellY = frameCellOffsetY + y;
        for (size_t x = 0; x < frameData.nbCellsX; x++)
        {
            const size_t curCellX = frameCellOffsetX + x;

            const size_t curPbCellIndex    = curCellX + curCellY * data.nbPixelBufferCellsX;
            const size_t curFrameCellIndex = x + y * frameData.nbCellsX;

            size_t& lastPixelEntryIndexForCell = pixelBuffer[curPbCellIndex];

            bool    sameAsPreviousCell = false; // By default always decode the cell
            uint8_t pixelMask          = 0x0F;  // Default pixel mask

            // Check if this cell is equal to the previous one
            if (lastPixelEntryIndexForCell < pbEntries.size()) {
                // Check if we have to reuse the previous values
                if (data.dirRef.header.compressEqualCells) {
                    // If true, the cell is the same as the previous one or transparent.
                    // Which actually mean the same thing : skip the decoding of this cell
                    sameAsPreviousCell = data.equalCellBitStream.readBool();
                }
                if (!sameAsPreviousCell) {
                    pixelMask = data.pixelMaskBitStream.readUnsigned8OrLess(4);
                }
            }

            // Store the fact that we skipped this cell for the 2nd phase
            frameData.cellSameAsPrevious[curFrameCellIndex] = sameAsPreviousCell;

            if (!sameAsPreviousCell) {
                // Pixel buffer entries are encoded as a stack in the stream which means the
                // last value decoded is actually the 1st value with a bit in the mask.
                PixelCodesStack pixelCodesStack = {};
                int nbPixelsDecoded = decodePixelCodesStack(data, pixelMask, pixelCodesStack);

                PixelBufferEntry previousEntryForCell;
                if (lastPixelEntryIndexForCell < pbEntries.size()) {
                    previousEntryForCell = pbEntries[lastPixelEntryIndexForCell];
                }
                else
                {
                    // No need for previousEntryForCell if it doesn't exist, as the mask is 0xF
                    assert(pixelMask == 0xF);
                }

                // Finalize the decoding of the pixel buffer entry
                PixelBufferEntry newEntry;

                int curIndex = nbPixelsDecoded - 1;
                for (size_t i = 0; i < PixelBufferEntry::nbValues; i++)
                {
                    // Pop a value if bit set in the mask
                    if (pixelMask & (1u << i)) {
                        uint8_t pixelCode;
                        if (curIndex >= 0) {
                            pixelCode = pixelCodesStack[size_t(curIndex--)];
                        }
                        else
                        {
                            pixelCode = 0;
                        }
                        // Store the actual values instead of the codes
                        newEntry.values[i] = data.codeToPixelValue[pixelCode];
                    }
                    // If not set, use the previous value for this entry
                    else
                    {
                        newEntry.values[i] = previousEntryForCell.values[i];
                    }
                }
                // Update the pixel buffer cell information
                lastPixelEntryIndexForCell = pbEntries.size();
                // Add the new entry for use in the 2nd stage
                pbEntries.push_back(newEntry);
            }
        }
    }
}

void decodeDirectionStage1(DirectionData& data, Vector<PixelBufferEntry>& pbEntries)
{
    // For each cell store a PixelBufferEntry index that points to the last entry for this cell
    // This will be used to retrieve values from the previous frame
    constexpr size_t    invalidIndex       = std::numeric_limits<size_t>::max();
    const size_t        pixelBufferNbCells = data.nbPixelBufferCellsX * data.nbPixelBufferCellsY;
    Vector<size_t>      pixelBuffer(pixelBufferNbCells, invalidIndex);

    // 1st phase of decoding : fill the pixel buffer
    // We actually fill a buffer of entries as to avoid storing empty entries
    for (size_t frameIndex = 0; frameIndex < data.nbFrames; ++frameIndex)
    {
        FrameData& frameData            = data.framesData[frameIndex];
        frameData.firstPixelBufferEntry = pbEntries.size();
        decodeFrameStage1(data, frameData, pixelBuffer, pbEntries);
    }
}

void decodeDirectionStage2(DirectionData& data, const Vector<PixelBufferEntry>& pbEntries)
{
    // This is the reason why we need to stages, we don't have the offset of this bitstream
    BitStreamView& pixelCodeIndices = data.pixelCodesDisplacementBitStream;

    const size_t pbWidth            = size_t(data.dirRef.extents.width());
    const size_t pbHeight           = size_t(data.dirRef.extents.height());
    const size_t pbStride           = pbWidth;
    const size_t nbPixelBufferCells = data.nbPixelBufferCellsX * data.nbPixelBufferCellsY;

    Vector<Cell>       pixelBufferCells(nbPixelBufferCells, Cell{0xF, 0xF});
    Vector<uint8_t>    pixelBufferColors(pbStride * pbHeight);
    ImageView<uint8_t> pBuffer{pixelBufferColors.data(), pbWidth, pbHeight, pbStride};

    // 2nd phase of decoding : Finish using the pixel buffer entries
    for (size_t frameIndex = 0; frameIndex < data.nbFrames; ++frameIndex)
    {
        const FrameData& frameData    = data.framesData[frameIndex];
        size_t           pbEntryIndex = frameData.firstPixelBufferEntry;

        size_t pbCellPosY = frameData.offsetY;
        for (size_t cellY = 0; cellY < frameData.nbCellsY; cellY++)
        {
            size_t pbCellPosX = frameData.offsetX;
            for (size_t cellX = 0; cellX < frameData.nbCellsX; cellX++)
            {
                const size_t frameCellIndex = cellX + cellY * frameData.nbCellsX;
                Cell         frameCell;
                frameCell.width  = frameData.cellWidths[cellX];
                frameCell.height = frameData.cellHeights[cellY];

                const size_t pbCellIndex =
                    (pbCellPosX / pbCellMaxPixelSize) +
                    (pbCellPosY / pbCellMaxPixelSize) * data.nbPixelBufferCellsX;
                Cell& pbCell = pixelBufferCells[pbCellIndex];

                if (frameData.cellSameAsPrevious[frameCellIndex]) {
                    if ((frameCell.width != pbCell.width) || (frameCell.height != pbCell.height)) {
                        // Clear the cell to 0
                        // If we used the previous frame pixels instead of reusing the same
                        // buffer for all frames we would not need to clear this (it would be
                        // initialized to 0 before writing any value to the frame pixels) But we
                        // would then need to copy values if the size matched
                        pBuffer.fillBytes(pbCellPosX, pbCellPosY, frameCell.width, frameCell.height,
                                          0);
                    }
                    else
                    {
                        // Same size, copy from previous cell in the buffer
                        // Nothing to change !
                    }
                }
                else
                {
                    const auto& pixelValues = pbEntries[pbEntryIndex++].values;

                    if (pixelValues[0] == pixelValues[1]) {
                        // This means we only got one pixel code, so fill the cell with it
                        pBuffer.fillBytes(pbCellPosX, pbCellPosY, frameCell.width, frameCell.height,
                                          pixelValues[0]);
                    }
                    else
                    {
                        int nbBitsToRead = 0;
                        if (pixelValues[1] == pixelValues[2]) {
                            // Stopped decoding after the 2nd value, only pixelValues[0] and
                            // pixelValues[1] are different, so only 1bit needs to be read to choose
                            // from those values
                            nbBitsToRead = 1;
                        }
                        else // We need 2 bits to index 3-4 values
                        {
                            nbBitsToRead = 2;
                        }

                        // fill FRAME cell with pixels
                        for (size_t y = 0; y < frameCell.height; y++)
                        {
                            for (size_t x = 0; x < frameCell.width; x++)
                            {
                                const uint8_t pixelCodeIndex =
                                    pixelCodeIndices.readUnsigned8OrLess(nbBitsToRead);
                                // Note: This actually means that a cell (4x4 block) can use at most
                                // 4 colors, a bit like DXT !
                                const uint8_t pixelValue = pixelValues[pixelCodeIndex];

                                pBuffer(pbCellPosX + x, pbCellPosY + y) = pixelValue;
                            }
                        }
                    }
                }

                pbCell = frameCell;
                pbCellPosX += frameCell.width;
            }
            pbCellPosY += frameData.cellHeights[cellY];
        }
        // Done decoding this frame, now copy its content.
        const ImageView<uint8_t> frameImageView = frameData.imageView;
        const ImageView<uint8_t> pbFrameView    = pBuffer.subView(
            frameData.offsetX, frameData.offsetY, frameImageView.width, frameImageView.height);
        assert(frameImageView.isValid() && pbFrameView.isValid());

        pbFrameView.copyTo(frameImageView);

/// Set to 1 to export the frames to the grayscale PPM format
#define DEBUG_EXPORT_PPM 0
#if DEBUG_EXPORT_PPM
/// Set to export all the frames at the size of the pixel buffer (not cleared outside of the frame)
#define EXPORT_FULL_SIZE false
        auto filename = fmt::format("test{}.ppm", frameIndex);
        Utils::exportToPGM(filename.c_str(), EXPORT_FULL_SIZE ? pBuffer : frameImageView);
#endif
    }
}
} // anonymous namespace
bool DCC::readDirection(Direction& outDir, uint32_t dirIndex, IImageProvider<uint8_t>& imgProvider)
{
    const size_t    directionEncodedSize = getDirectionSize(dirIndex);
    Vector<uint8_t> buffer(directionEncodedSize);
    stream->seek(directionsOffsets[dirIndex], IStream::beg);
    stream->read(buffer.data(), directionEncodedSize);
    assert(stream->good());
    BitStreamView bitStream(buffer.data(), directionEncodedSize * CHAR_BIT);

    DirectionHeader& dirHeader = outDir.header;
    if (!readDirHeader(dirHeader, bitStream)) return false;

    if (!readFrameHeaders(header.framesPerDir, outDir, bitStream)) return false;

    outDir.computeDirExtents();

    DirectionData data{outDir, bitStream, header.framesPerDir, imgProvider};
    if (!data.isValid()) return false;

    Vector<PixelBufferEntry> pbEntries;
    {
        size_t estimatedNbEntries =
            (header.framesPerDir * data.nbPixelBufferCellsX * data.nbPixelBufferCellsY) / 4;
        pbEntries.reserve(estimatedNbEntries);
    }

    decodeDirectionStage1(data, pbEntries);

    decodeDirectionStage2(data, pbEntries);

    // Make sure we fully read the streams
    assert(data.equalCellBitStream.tell() == data.equalCellBitStream.sizeInBits());
    assert(data.pixelMaskBitStream.tell() == data.pixelMaskBitStream.sizeInBits());
    assert(data.rawPixelUsageBitStream.tell() == data.rawPixelUsageBitStream.sizeInBits());
    assert(data.rawPixelCodesBitStream.tell() == data.rawPixelCodesBitStream.sizeInBits());
    // This exact stream size is not known, so check if we at least are in the last byte
    assert(data.pixelCodesDisplacementBitStream.bitPositionInBuffer() + 7_z >=
           data.pixelCodesDisplacementBitStream.bufferSizeInBits());

    return bitStream.good();
}

} // namespace WorldStone

/// @endinternal
