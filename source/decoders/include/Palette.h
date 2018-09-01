//
// Created by Lectem on 06/11/2016.
//

#pragma once

#include <cstdint>
#include <Stream.h>
#include <array>

namespace WorldStone
{
/**
 * @brief Helper to load a Diablo 2 palette (.pal/.dat format)
 *
 * Diablo II uses 256 colors palettes to render images.
 * Most of the time they are used in combination with colormaps to have different color variations
 * of a same image.
 *
 */
struct Palette
{
    struct Color
    {
        uint8_t r, g, b, _padding;
        bool operator==(const Color& rhs) const { return r == rhs.r && g == rhs.g && b == rhs.b; }
    };

    struct Color24Bits
    {
        uint8_t r, g, b;
        bool operator==(const Color24Bits& rhs) const
        {
            return r == rhs.r && g == rhs.g && b == rhs.b;
        }
    };
    static const size_t colorCount = 256;
    std::array<Color, colorCount> colors;

    bool decode(const char* filename);
    bool decode(IStream* file);
    uint8_t GetClosestColorIndex(Color color);
    bool operator==(const Palette& rhs) const { return colors == rhs.colors; }
};

struct PalShiftTransform
{
    std::array<uint8_t, Palette::colorCount> indices;

    Palette::Color GetTranformedColor(const Palette& palette, uint8_t colorIndex) const
    {
        return palette.colors[indices[colorIndex]];
    }

    void GetTranformedPalette(Palette& outputPalette, const Palette& basePalette) const
    {
        for (size_t i = 0; i < Palette::colorCount; i++)
        {
            outputPalette.colors[i] = basePalette.colors[indices[i]];
        }
    }
};

/**
 * @brief Precomputed palette variations in the form of palette shifts
 */
struct PL2
{
    Palette basePalette;

    PalShiftTransform lightLevelVariations[32];
    PalShiftTransform invColorVariations[16];
    PalShiftTransform selectedUnitShift; ///< Needs confirmation for usage

    PalShiftTransform alphaBlend[3][256]; ///< glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    PalShiftTransform additiveBlend[256]; ///< glBlendFunc(GL_ONE, GL_ONE)
    PalShiftTransform multiplicativeBlend[256]; ///< glBlendFunc(GL_ZERO, GL_SRC_COLOR)

    PalShiftTransform hueVariations[111];
    PalShiftTransform redTones;
    PalShiftTransform greenTones;
    PalShiftTransform blueTones;
    PalShiftTransform unknownColorVariations[14];

    PalShiftTransform maxComponentBlend[256];
    PalShiftTransform darkenedColorShift;

    Palette::Color24Bits textColors[13];
    PalShiftTransform    textColorShifts[13];

    static std::unique_ptr<PL2> CreateFromPalette(const Palette& palette);
    static std::unique_ptr<PL2> ReadFromStream(IStream* stream);
};

} // namespace WorldStone
