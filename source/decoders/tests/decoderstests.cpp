/**
 * @file decoderstests.cpp
 * @brief Implementation of the tests for the various file decoders.
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <dcc.h>
#include "doctest.h"
using WorldStone::DCC;
using WorldStone::SimpleImageProvider;
using WorldStone::FileStream;

/**Try to decode BaalSpirit.dcc.
 * This is the DCC file with the biggest number of frames (but only 1 direction).
 * @testimpl{WorldStone::DCC,DCC_BaalSpirit}
 */
TEST_CASE("DCC decoding BaalSpirit.dcc")
{
    DCC dcc;
    REQUIRE(dcc.initDecoder(std::make_unique<FileStream>("BaalSpirit.dcc")));
    // clang-format off
    const DCC::Header& header = dcc.getHeader();

    CHECK(header.signature    ==    116);
    CHECK(header.version      ==      6);
    CHECK(header.directions   ==      1);
    CHECK(header.framesPerDir ==    200);
    CHECK(header.padding0[0]  ==      0);
    CHECK(header.padding0[1]  ==      0);
    CHECK(header.padding0[2]  ==      0);
    CHECK(header.tag          ==      1);
    CHECK(header.finalDc6Size == 547757);

    DCC::Direction dir0;
    SimpleImageProvider<uint8_t> imgProvider;
    REQUIRE(dcc.readDirection(dir0, 0, imgProvider));

    CHECK(dir0.header.outsizeCoded        ==  546933);
    CHECK(dir0.header.hasRawPixelEncoding ==   false);
    CHECK(dir0.header.compressEqualCells  ==    true);
    CHECK(dir0.header.variable0Bits       ==       0);
    CHECK(dir0.header.widthBits           ==       5);
    CHECK(dir0.header.heightBits          ==       5);
    CHECK(dir0.header.xOffsetBits         ==       5);
    CHECK(dir0.header.yOffsetBits         ==       5);
    CHECK(dir0.header.optionalBytesBits   ==       0);
    CHECK(dir0.header.codedBytesBits      ==       9);

    CHECK(dir0.extents.xLower   ==     -51);
    CHECK(dir0.extents.yLower   ==    -178);
    CHECK(dir0.extents.xUpper   == (143+1));
    CHECK(dir0.extents.yUpper   ==  (45+1));
    CHECK(dir0.extents.width()  ==     195);
    CHECK(dir0.extents.height() ==     224);
    // clang-format on
}

/**@testimpl{WorldStone::DCC,DCC_CRHDBRVDTHTH}
 * This test makes sure we test the case where a 0b0000 mask is used for a cell.
 */
TEST_CASE("DCC decoding CRHDBRVDTHTH.dcc")
{
    DCC dcc;
    REQUIRE(dcc.initDecoder(std::make_unique<FileStream>("CRHDBRVDTHTH.dcc")));
    // clang-format off
    const DCC::Header& header = dcc.getHeader();

    CHECK(header.signature    ==   116);
    CHECK(header.version      ==     6);
    CHECK(header.directions   ==     8);
    CHECK(header.framesPerDir ==    24);
    CHECK(header.padding0[0]  ==     0);
    CHECK(header.padding0[1]  ==     0);
    CHECK(header.padding0[2]  ==     0);
    CHECK(header.tag          ==     1);
    CHECK(header.finalDc6Size == 59600);

    DCC::Direction dir0;
    SimpleImageProvider<uint8_t> imgProvider;
    REQUIRE(dcc.readDirection(dir0, 0, imgProvider));

    CHECK(dir0.header.outsizeCoded        ==  7225);
    CHECK(dir0.header.hasRawPixelEncoding ==  true);
    CHECK(dir0.header.compressEqualCells  ==  true);
    CHECK(dir0.header.variable0Bits       ==     0);
    CHECK(dir0.header.widthBits           ==     4);
    CHECK(dir0.header.heightBits          ==     4);
    CHECK(dir0.header.xOffsetBits         ==     4);
    CHECK(dir0.header.yOffsetBits         ==     5);
    CHECK(dir0.header.optionalBytesBits   ==     0);
    CHECK(dir0.header.codedBytesBits      ==     6);

    CHECK(dir0.extents.xLower   ==     -12);
    CHECK(dir0.extents.yLower   ==     -75);
    CHECK(dir0.extents.xUpper   ==  (46+1));
    CHECK(dir0.extents.yUpper   == (-15+1));
    CHECK(dir0.extents.width()  ==      59);
    CHECK(dir0.extents.height() ==      61);
    // clang-format on
}

/**@testimpl{WorldStone::DCC,DCC_BloodSmall01}
 * This test makes sures that we treat the case of the 2nd cell
 * being merged into the 1st one correctly when it has only 1 pixel.
 */
TEST_CASE("DCC decoding BloodSmall01.dcc")
{
    DCC dcc;
    REQUIRE(dcc.initDecoder(std::make_unique<FileStream>("BloodSmall01.dcc")));
    // clang-format off
    const DCC::Header& header = dcc.getHeader();

    CHECK(header.signature    ==   116);
    CHECK(header.version      ==     6);
    CHECK(header.directions   ==     8);
    CHECK(header.framesPerDir ==     9);
    CHECK(header.padding0[0]  ==     0);
    CHECK(header.padding0[1]  ==     0);
    CHECK(header.padding0[2]  ==     0);
    CHECK(header.tag          ==     1);
    CHECK(header.finalDc6Size == 14200);

    DCC::Direction dir0;
    SimpleImageProvider<uint8_t> imgProvider;
    REQUIRE(dcc.readDirection(dir0, 0, imgProvider));

    CHECK(dir0.header.outsizeCoded        ==  1776);
    CHECK(dir0.header.hasRawPixelEncoding == false);
    CHECK(dir0.header.compressEqualCells  == false);
    CHECK(dir0.header.variable0Bits       ==     0);
    CHECK(dir0.header.widthBits           ==     4);
    CHECK(dir0.header.heightBits          ==     4);
    CHECK(dir0.header.xOffsetBits         ==     4);
    CHECK(dir0.header.yOffsetBits         ==     5);
    CHECK(dir0.header.optionalBytesBits   ==     0);
    CHECK(dir0.header.codedBytesBits      ==     6);

    CHECK(dir0.extents.xLower   ==     -18);
    CHECK(dir0.extents.yLower   ==     -44);
    CHECK(dir0.extents.xUpper   ==  (13+1));
    CHECK(dir0.extents.yUpper   ==   (9+1));
    CHECK(dir0.extents.width()  ==      32);
    CHECK(dir0.extents.height() ==      54);
    // clang-format on
}

/**@testimpl{WorldStone::DCC,DCC_HZTRLITA1HTH}
 *Special has it has a 1bit value for an offset that gives - 1.
 */
TEST_CASE("DCC decoding HZTRLITA1HTH.dcc")
{
    DCC dcc;
    REQUIRE(dcc.initDecoder(std::make_unique<FileStream>("HZTRLITA1HTH.dcc")));
    // clang-format off
    const DCC::Header& header = dcc.getHeader();

    CHECK(header.signature    ==   116);
    CHECK(header.version      ==     6);
    CHECK(header.directions   ==     8);
    CHECK(header.framesPerDir ==    12);
    CHECK(header.padding0[0]  ==     0);
    CHECK(header.padding0[1]  ==     0);
    CHECK(header.padding0[2]  ==     0);
    CHECK(header.tag          ==     1);
    CHECK(header.finalDc6Size == 92732);

    DCC::Direction dir;
    SimpleImageProvider<uint8_t> imgProvider;
    REQUIRE(dcc.readDirection(dir, 0, imgProvider));

    CHECK(dir.header.outsizeCoded        == 10923);
    CHECK(dir.header.hasRawPixelEncoding == false);
    CHECK(dir.header.compressEqualCells  ==  true);
    CHECK(dir.header.variable0Bits       ==     0);
    CHECK(dir.header.widthBits           ==     4);
    CHECK(dir.header.heightBits          ==     4);
    CHECK(dir.header.xOffsetBits         ==     4);
    CHECK(dir.header.yOffsetBits         ==     1);
    CHECK(dir.header.optionalBytesBits   ==     0);
    CHECK(dir.header.codedBytesBits      ==     6);

    CHECK(dir.extents.xLower   ==     -22);
    CHECK(dir.extents.yLower   ==     -62);
    CHECK(dir.extents.xUpper   ==  (33+1));
    CHECK(dir.extents.yUpper   ==  (-1+1));
    CHECK(dir.extents.width()  ==      56);
    CHECK(dir.extents.height() ==      62);

    REQUIRE(dcc.readDirection(dir, 4, imgProvider));

    CHECK(dir.header.outsizeCoded        ==  9774);
    CHECK(dir.header.hasRawPixelEncoding == false);
    CHECK(dir.header.compressEqualCells  ==  true);
    CHECK(dir.header.variable0Bits       ==     0);
    CHECK(dir.header.widthBits           ==     4);
    CHECK(dir.header.heightBits          ==     4);
    CHECK(dir.header.xOffsetBits         ==     1);
    CHECK(dir.header.yOffsetBits         ==     1);
    CHECK(dir.header.optionalBytesBits   ==     0);
    CHECK(dir.header.codedBytesBits      ==     6);

    CHECK(dir.extents.xLower   ==      -1);
    CHECK(dir.extents.yLower   ==     -62);
    CHECK(dir.extents.xUpper   ==  (22+1));
    CHECK(dir.extents.yUpper   ==  (-1+1));
    CHECK(dir.extents.width()  ==      24);
    CHECK(dir.extents.height() ==      62);
    // clang-format on
}
