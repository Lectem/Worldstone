/**
 * @file SystemUtilsTests.cpp
 */
#include <SystemUtils.h>
#include <doctest.h>

using WorldStone::Utils::signExtend;
/**Test that SignExtend is giving the right values
 * @testimpl{WorldStone::Utils::signExtend(),SignExtend}
 */
TEST_CASE("SignExtend")
{
    // clang-format off
    SUBCASE("Positive input")
    {
        CHECK(signExtend<int32_t,13>(         0) ==          0);
        CHECK(signExtend<int32_t,32>(0x16641337) == 0x16641337);
        CHECK(signExtend<int32_t,32>(0x7FFFFFFF) == 0x7FFFFFFF);
        CHECK(signExtend<int32_t, 2>(      0b01) ==          1);
        CHECK(signExtend<int32_t, 3>(     0b011) ==          3);
    }
    SUBCASE("Negative input")
    {
        CHECK(signExtend<int32_t, 5>(   0b10110) ==        -10);
        CHECK(signExtend<int32_t, 8>(      0xFF) ==         -1);
        CHECK(signExtend<int32_t,32>(0xFFFFFFFF) ==         -1);
        CHECK(signExtend<int32_t,32>(0xDEADBEEF) == 0xDEADBEEF);
        CHECK(signExtend<int32_t,16>(0xDEADBEEF) == 0xFFFFBEEF);
        CHECK(signExtend<int32_t,16>(0x16641337) ==     0x1337);
        CHECK(signExtend<int32_t, 8>(      0x80) ==       -128);
        CHECK(signExtend<int32_t, 3>(      0x07) ==         -1);
        CHECK(signExtend<int32_t, 9>(     0x1CE) ==        -50);
    }
    SUBCASE("1-bit special case")
    {
        CHECK(signExtend<int32_t, 1>(0b0) ==  0);
        CHECK(signExtend<int32_t, 1>(0b1) == -1); // Sign extension => 0xFFFFFFFF => -1
    }
    // clang-format on
}

using WorldStone::Utils::reverseBits;
/// @testimpl{WorldStone::Utils::reverseBits(),ReverseBits}
TEST_CASE("ReverseBits")
{
    CHECK(reverseBits<uint8_t>(0xF0) == 0x0F);
    CHECK(reverseBits<uint16_t>(0xF0) == 0x0F00);
    CHECK(reverseBits<uint32_t>(0xF0) == 0x0F000000);
    CHECK(reverseBits<uint64_t>(0xF0) == 0x0F00000000000000);
    CHECK(reverseBits<uint32_t>(0x01234567) == 0xE6A2C480);
}

using WorldStone::Utils::popCount;
/**Test that popCount is giving the right values
 * @testimpl{WorldStone::Utils::popCount(),PopCount}
 */
TEST_CASE("PopCount 16bits")
{
    CHECK(popCount(uint16_t(0b0000000000000000)) == 0);
    CHECK(popCount(uint16_t(0b0000000000011111)) == 5);
    CHECK(popCount(uint16_t(0b1110011101011001)) == 10);
    CHECK(popCount(uint16_t(0b1111111111111111)) == 16);

    CHECK(popCount(uint32_t(0x00000000)) == 0);
    CHECK(popCount(uint32_t(0x000F000F)) == 8);
    CHECK(popCount(uint32_t(0x0F000F00)) == 8);
    CHECK(popCount(uint32_t(0xFFFFFFFF)) == 32);

    CHECK(popCount(uint64_t(0x0000000000000000)) == 0);
    CHECK(popCount(uint64_t(0x00000000000000FF)) == 8);
    CHECK(popCount(uint64_t(0x000000000000FF00)) == 8);
    CHECK(popCount(uint64_t(0xFFFFFFFFFFFFFFFF)) == 64);
}
