/**
 * @file PaletteTests.cpp
 * @brief Implementation of the tests for Palette and PL2
 */
#include <FileStream.h>
#include <palette.h>
#include <Platform.h>
#include <SystemUtils.h>
#include <doctest.h>

using WorldStone::Palette;
using WorldStone::PL2;
using WorldStone::FileStream;

TEST_CASE("PL2 creation")
{
    Palette basePalette;
    bool    ok = basePalette.decode("pal.dat");
    REQUIRE(ok);

    std::unique_ptr<PL2> pl2 = PL2::CreateFromPalette(basePalette);
    FileStream           fs("pal.pl2");
    REQUIRE(fs.good());
    std::unique_ptr<PL2> pl2File = std::make_unique<PL2>();
    fs.read(pl2File.get(), sizeof(PL2));

    bool sameBase = pl2->basePalette == pl2File->basePalette;
    REQUIRE(basePalette == pl2File->basePalette);
    CHECK(pl2->basePalette == pl2File->basePalette);
    for (size_t i = 0; i < 32; i++)
        CHECK(pl2->lightLevelVariations[i].indices == pl2File->lightLevelVariations[i].indices);
    for (size_t i = 0; i < 16; i++)
        CHECK(pl2->invColorVariations[i].indices == pl2File->invColorVariations[i].indices);

    CHECK(pl2->selectedUnitShift.indices == pl2File->selectedUnitShift.indices);
    for (size_t blendLevel = 0; blendLevel < 3; blendLevel++)
        for (size_t i = 0; i < 256; i++)
            CHECK(pl2->alphaBlend[blendLevel][i].indices
                  == pl2File->alphaBlend[blendLevel][i].indices);

    for (size_t i = 0; i < 256; i++)
        CHECK(pl2->additiveBlend[i].indices == pl2File->additiveBlend[i].indices);

    for (size_t i = 0; i < 256; i++)
        CHECK(pl2->multiplicativeBlend[i].indices == pl2File->multiplicativeBlend[i].indices);

    for (size_t i = 0; i < 111; i++)
        CHECK(pl2->hueVariations[i].indices == pl2File->hueVariations[i].indices);

    // Probably unsused ? Always 0
    for (size_t i = 0; i < 14; i++)
        CHECK(pl2->unknownColorVariations[i].indices == pl2File->unknownColorVariations[i].indices);

    CHECK(pl2->redTones.indices == pl2File->redTones.indices);
    CHECK(pl2->greenTones.indices == pl2File->greenTones.indices);
    CHECK(pl2->blueTones.indices == pl2File->blueTones.indices);

    for (size_t i = 0; i < 256; i++)
        CHECK(pl2->maxComponentBlend[i].indices == pl2File->maxComponentBlend[i].indices);

    CHECK(pl2->darkenedColorShift.indices == pl2File->darkenedColorShift.indices);
    for (size_t i = 0; i < 13; i++)
    {
        CHECK(pl2->textColors[i] == pl2File->textColors[i]);
        CHECK(pl2->textColorShifts[i].indices == pl2File->textColorShifts[i].indices);
    }
}
