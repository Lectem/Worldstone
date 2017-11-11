#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <dcc.h>
#include "doctest.h"
using WorldStone::DCC;

/**
 * @cond TEST
 */

TEST_CASE("DCC decoding")
{
    DCC dcc;
    CHECK(dcc.decode("BaalSpirit.dcc"));
    // clang-format off
    const DCC::Header& header = dcc.getHeader();
    CHECK_EQ(header.signature,         116);
    CHECK_EQ(header.version,             6);
    CHECK_EQ(header.directions,          1);
    CHECK_EQ(header.framesPerDir,      200);
    CHECK_EQ(header.padding0[0],         0);
    CHECK_EQ(header.padding0[1],         0);
    CHECK_EQ(header.padding0[2],         0);
    CHECK_EQ(header.tag,                 1);
    CHECK_EQ(header.finalDc6Size,   547757);

    DCC::Direction dir0;
    CHECK(dcc.readDirection(dir0, 0));

    CHECK_EQ(dir0.header.outsizeCoded          ,  546933);
    CHECK_EQ(dir0.header.compressColorEncoding ,   false);
    CHECK_EQ(dir0.header.compressEqualCells    ,    true);
    CHECK_EQ(dir0.header.variable0Bits         ,       0);
    CHECK_EQ(dir0.header.widthBits             ,       5);
    CHECK_EQ(dir0.header.heightBits            ,       5);
    CHECK_EQ(dir0.header.xoffsetBits           ,       5);
    CHECK_EQ(dir0.header.yoffsetBits           ,       5);
    CHECK_EQ(dir0.header.optionalBytesBits     ,       0);
    CHECK_EQ(dir0.header.codedBytesBits        ,       9);

    CHECK_EQ(dir0.extents.xMin, -51);
    CHECK_EQ(dir0.extents.yMin,-178);
    CHECK_EQ(dir0.extents.xMax, 143);
    CHECK_EQ(dir0.extents.yMax,  45);
    // clang-format on
}

/**
 * @endcond TEST
 */