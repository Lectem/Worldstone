/**
 * @file BitStreamTests.cpp
 */
#include <BitStream.h>
#include <SystemUtils.h>
#include "doctest.h"

using WorldStone::BitStreamView;

/**Test the Bitstream usage in read-only.
 * @testimpl{WorldStone::BitStreamView,RO_bitstream}
 */
TEST_CASE("BitStreamView read.")
{
    const char buffer[] = {0x01, 0x23, 0x45, 0x67, (char)0x89, (char)0xAB, (char)0xCD, (char)0xEF};
    BitStreamView bitstream{buffer, sizeof(buffer) * CHAR_BIT};
    CHECK(bitstream.bufferSizeInBytes() == sizeof(buffer));
    CHECK(bitstream.sizeInBits() == sizeof(buffer) * CHAR_BIT);
    SUBCASE("Unsigned integer reads")
    {
        // clang-format off
        CHECK_EQ(bitstream.readUnsigned< 0>() , /*==*/ ( 0                       ));
        CHECK_EQ(bitstream.readUnsigned< 8>() , /*==*/ ( 0x01                    ));
        CHECK_EQ(bitstream.readUnsigned<16>() , /*==*/ ( 0x4523                  )); // Little endian
        CHECK_EQ(bitstream.readUnsigned< 3>() , /*==*/ ( 0x67            & 0b111 ));
        CHECK_EQ(bitstream.readUnsigned<13>() , /*==*/ ( 0x8967   >> 3           ));
        CHECK_EQ(bitstream.readUnsigned<13>() , /*==*/ ( 0xCDAB          & 0x1FFF));
        CHECK_EQ(bitstream.readUnsigned< 2>() , /*==*/ ((0xCDAB   >> 13) & 0b11  ));
        CHECK_EQ(bitstream.readUnsigned< 9>() , /*==*/ ( 0xEFCDAB >> 15          ));
        // clang-format on
    }
    // Test the unsigned integers read with small return types
    SUBCASE("Signed integer reads")
    {
        // clang-format off
        CHECK(bitstream.readUnsigned< 0,  uint8_t>() ==  uint8_t(     0));
        CHECK(bitstream.readUnsigned< 8,  uint8_t>() ==  uint8_t(  0x01));
        CHECK(bitstream.readUnsigned< 8,  uint8_t>() ==  uint8_t(  0x23));
        CHECK(bitstream.readUnsigned< 8,  uint8_t>() ==  uint8_t(  0x45));
        CHECK(bitstream.readUnsigned<16, uint16_t>() == uint16_t(0x8967));
        // clang-format on
    }
    // Test the read 0/1 bits
    SUBCASE("0 and 1 bits reads")
    {
        CHECK(bitstream.readBit());
        CHECK_FALSE(bitstream.readBit());
        CHECK(bitstream.tell() == 2);
        CHECK(bitstream.read0Bits() == 0);
        CHECK(bitstream.tell() == 2);
        // Test the signed integers read
        bitstream.skip(6);
        CHECK(bitstream.readSigned<0>() == 0);
        CHECK(bitstream.readSigned<1>() == -1);

        CHECK(bitstream.tell() == 2 + 6 + 1);
    }
    SUBCASE("Aligning to byte")
    {
        // Check if we align to the next byte correctly
        bitstream.setPosition(9);
        bitstream.alignToByte();
        CHECK(bitstream.tell() == 16);
        // Check that we do not change the position if already aligned
        bitstream.alignToByte();
        CHECK(bitstream.tell() == 16);
    }

    SUBCASE("Subview of same size")
    {
        BitStreamView subView = bitstream.createSubView(bitstream.sizeInBits());
        CHECK(subView.sizeInBits() == bitstream.sizeInBits());
        CHECK(subView.tell() == bitstream.tell());
    }
    SUBCASE("Subview of same size and offset in first byte")
    {
        bitstream.setPosition(5);
        BitStreamView subView = bitstream.createSubView(bitstream.sizeInBits() - 5);
        CHECK(subView.bufferSizeInBytes() == bitstream.bufferSizeInBytes()); // Didn't change bytes
        CHECK(subView.sizeInBits() == bitstream.sizeInBits() - 5);           // Didn't change bytes
        CHECK(subView.tell() == 0);
    }
    SUBCASE("Subview of same size and offset in 2nd byte")
    {
        const size_t offset = CHAR_BIT + 2;
        bitstream.setPosition(offset);
        BitStreamView subView = bitstream.createSubView(bitstream.sizeInBits() - offset);
        CHECK(subView.bufferSizeInBytes() ==
              bitstream.bufferSizeInBytes() - 1); // We're in the second byte
        CHECK(subView.tell() == 0);
    }
    SUBCASE("Subview of size 0")
    {
        bitstream.setPosition(rand() % bitstream.sizeInBits());
        BitStreamView subView = bitstream.createSubView(0);
        CHECK(subView.bufferSizeInBytes() == 0); // We're in the second byte
        CHECK(subView.tell() == 0);
    }
    CHECK(bitstream.good());
    // TODO : set badbit on failure
}
