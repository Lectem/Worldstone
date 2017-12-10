/**
 * @file decoderstests.cpp
 * @brief Implementation of the tests for the various file decoders.
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <dcc.h>
#include "doctest.h"
using WorldStone::DCC;
using WorldStone::SimpleImageProvider;

/**Try to decode BaalSpirit.dcc.
 * This is the DCC file with the biggest number of frames (but only 1 direction).
 * @testimpl{WorldStone::DCC,DCC_BaalSpirit}
 */
TEST_CASE("DCC decoding BaalSpirit.dcc")
{
    DCC dcc;
    REQUIRE(dcc.decode("BaalSpirit.dcc"));
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
    CHECK(dir0.header.xoffsetBits         ==       5);
    CHECK(dir0.header.yoffsetBits         ==       5);
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
