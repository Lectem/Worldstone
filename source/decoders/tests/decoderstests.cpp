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
    CHECK(dcc.Decode("BaalSpirit.dcc"));
    // clang-format off
    const DCC::Header& header = dcc.getHeader();
    CHECK_EQ(header.signature,         116);
    CHECK_EQ(header.version,             6);
    CHECK_EQ(header.directions,          1);
    CHECK_EQ(header.frames_per_dir,    200);
    CHECK_EQ(header.padding0[0],         0);
    CHECK_EQ(header.padding0[1],         0);
    CHECK_EQ(header.padding0[2],         0);
    CHECK_EQ(header.tag,                 1);
    CHECK_EQ(header.final_dc6_size, 547757);

    DCC::Direction dir0;
    CHECK(dcc.readDirection(dir0, 0));

    CHECK_EQ(dir0.header.outsize_coded         ,  546933);
    CHECK_EQ(dir0.header.compressColorEncoding ,   false);
    CHECK_EQ(dir0.header.compressEqualCells    ,    true);
    CHECK_EQ(dir0.header.variable0_bits        ,       0);
    CHECK_EQ(dir0.header.width_bits            ,       5);
    CHECK_EQ(dir0.header.height_bits           ,       5);
    CHECK_EQ(dir0.header.xoffset_bits          ,       5);
    CHECK_EQ(dir0.header.yoffset_bits          ,       5);
    CHECK_EQ(dir0.header.optional_bytes_bits   ,       0);
    CHECK_EQ(dir0.header.coded_bytes_bits      ,       9);

    CHECK_EQ(dir0.extents.xMin, -51);
    CHECK_EQ(dir0.extents.yMin,-178);
    CHECK_EQ(dir0.extents.xMax, 143);
    CHECK_EQ(dir0.extents.yMax,  45);
    // clang-format on
}

/**
 * @endcond TEST
 */