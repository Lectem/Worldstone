//
// Created by Lectem on 06/11/2016.
//

#include "Palette.h"
#include <Platform.h>
#include <stdio.h>
#include <FileStream.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <string.h>

WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wfloat-equal")
WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU("-Wmissing-field-initializers")

namespace WorldStone
{

bool Palette::decode(const char* filename)
{
    FileStream str{filename};
    return decode(&str);
}

bool Palette::decode(IStream* file)
{
    if (file && file->good())
    {
        for (size_t i = 0; i < colorCount; i++)
        {
            // order is BGR, not RGB
            colors[i].b        = static_cast<uint8_t>(file->getc());
            colors[i].g        = static_cast<uint8_t>(file->getc());
            colors[i].r        = static_cast<uint8_t>(file->getc());
            colors[i]._padding = 0;
        }
        return file->good();
    }
    return false;
}

uint8_t Palette::GetClosestColorIndex(Palette::Color color)
{
    uint32_t closestColorDistance = std::numeric_limits<uint32_t>::max();
    size_t   closestColorIndex    = 0;
    size_t   currentIndex         = 0;
    for (Color palColor : colors)
    {
        const int      diffRed   = palColor.r - color.r;
        const int      diffGreen = palColor.g - color.g;
        const int      diffBlue  = palColor.b - color.b;
        const uint32_t distance =
            uint32_t(diffRed * diffRed + diffGreen * diffGreen + diffBlue * diffBlue);
        if (distance < closestColorDistance)
        {
            closestColorDistance = distance;
            closestColorIndex    = currentIndex;
        }
        currentIndex++;
    }
    assert(closestColorIndex < 256);
    return uint8_t(closestColorIndex);
}

namespace
{

struct ColorHSL
{
    double hue; /// 0.0 to 360.0°
    double sat; /// 0.0 to 1.0
    double lum; /// 0.0 to 1.0
};

ColorHSL ConvertRGBtoHSL(const Palette::Color& rgb)
{
    ColorHSL     hsl;
    const double red       = rgb.r / 255.0;
    const double green     = rgb.g / 255.0;
    const double blue      = rgb.b / 255.0;
    const double minRGB    = std::min<double>({red, green, blue});
    const double maxRGB    = std::max({red, green, blue});
    const double minMaxSum = minRGB + maxRGB;

    hsl.lum = minMaxSum / 2.0;
    assert(hsl.lum >= 0.0 && hsl.lum <= 1.0);

    if (minRGB == maxRGB)
    {
        hsl.hue = 0.0;
        hsl.sat = 0.0;

        return hsl;
    }
    else
    {

        const double deltaMinMax = maxRGB - minRGB;
        if (hsl.lum > 0.5) { hsl.sat = deltaMinMax / (2.0 - minMaxSum); }
        else
        {
            hsl.sat = deltaMinMax / minMaxSum;
        }
        assert(hsl.sat >= 0.0 && hsl.sat <= 1.0);

        if (maxRGB == red) { hsl.hue = (green - blue) / deltaMinMax; }
        else if (maxRGB == green)
        {
            hsl.hue = (blue - red) / deltaMinMax + 2.0;
        }
        else // if (maxRGB == blue)
        {
            hsl.hue = (red - green) / deltaMinMax + 4.0;
        }
        hsl.hue *= 60.0; // Put the hue in degrees
        if (hsl.hue < 0.0) hsl.hue += 360.0;
        assert(hsl.hue >= 0 && hsl.hue < 360.0);
        return hsl;
    }
}

double hue2rgb(double p, double q, double t)
{
    if (t < 0.0) t += 360.0;
    if (t > 360.0) t -= 360.0;
    if (t < 60.0) return (q - p) * t / 60.0 + p;
    if (t < 180.0) return q;
    if (t < 240.0) return (q - p) * (240.0 - t) / 60.0 + p;
    return p;
}

Palette::Color ConvertHSLtoRGB(const ColorHSL& hsl)
{
    Palette::Color rgbColor;
    if (hsl.sat == 0.0)
    {
        rgbColor.r = rgbColor.g = rgbColor.b = uint8_t(hsl.lum * 255.0);
        return rgbColor;
    }
    else
    {
        double q;
        if (hsl.lum > 0.5)
            q = hsl.lum + hsl.sat - hsl.lum * hsl.sat;
        else
            q = (hsl.sat + 1.0) * hsl.lum;
        const double p = 2.0 * hsl.lum - q;

        rgbColor.r = uint8_t(hue2rgb(p, q, hsl.hue + 120.0) * 255.0);
        rgbColor.g = uint8_t(hue2rgb(p, q, hsl.hue) * 255.0);
        rgbColor.b = uint8_t(hue2rgb(p, q, hsl.hue - 120.0) * 255.0);
        return rgbColor;
    }
}
int GetAlphaBlendRatioFromLevel(int alphaBlendLevel)
{
    switch (alphaBlendLevel)
    {
    case 0: return 191;
    case 1: return 127;
    case 2: return 63;
    default: assert(false); return 0;
    }
}

void PL2CreateLightLevelVariations(PL2& pl2)
{

    for (size_t variationIndex = 0; variationIndex < 32; variationIndex++)
    {
        for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
        {
            const Palette::Color baseColor = pl2.basePalette.colors[colorIndex];
            const Palette::Color lightColor{
                uint8_t(((variationIndex + 1) * baseColor.r) >> 5),
                uint8_t(((variationIndex + 1) * baseColor.g) >> 5),
                uint8_t(((variationIndex + 1) * baseColor.b) >> 5),
            };
            pl2.lightLevelVariations[variationIndex].indices[colorIndex] =
                pl2.basePalette.GetClosestColorIndex(lightColor);
        }
    }
}

void PL2CreateInvColorVariations(PL2& pl2)
{

    for (size_t variationIndex = 0; variationIndex < Palette::colorCount; variationIndex++)
    {
        for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
        {
            const Palette::Color baseColor = pl2.basePalette.colors[colorIndex];
            const Palette::Color lightColor{
                uint8_t((((variationIndex + 1) * (255u - baseColor.r)) >> 4) + baseColor.r),
                uint8_t((((variationIndex + 1) * (255u - baseColor.g)) >> 4) + baseColor.g),
                uint8_t((((variationIndex + 1) * (255u - baseColor.b)) >> 4) + baseColor.b),
            };
            pl2.invColorVariations[variationIndex].indices[colorIndex] =
                pl2.basePalette.GetClosestColorIndex(lightColor);
        }
    }
}

void PL2CreateSelectedUnitShift(PL2& pl2, ColorHSL hslColors[Palette::colorCount])
{
    for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
    {

        ColorHSL tmpColorHSL = hslColors[colorIndex];
        if (tmpColorHSL.lum != 0.0) { tmpColorHSL.lum = std::min(tmpColorHSL.lum + 0.2, 1.0); }

        pl2.selectedUnitShift.indices[colorIndex] =
            pl2.basePalette.GetClosestColorIndex(ConvertHSLtoRGB(tmpColorHSL));
    }
}

void PL2CreateAlphaBlend(PL2& pl2)
{
    for (int alphaBlendLevel = 0; alphaBlendLevel < 3; alphaBlendLevel++)
    {
        const int alphaBlendRatio = GetAlphaBlendRatioFromLevel(alphaBlendLevel);
        for (size_t dstColorIndex = 0; dstColorIndex < Palette::colorCount; ++dstColorIndex)
        {
            const Palette::Color dstColor = pl2.basePalette.colors[dstColorIndex];
            for (size_t srcColorIndex = 0; srcColorIndex < Palette::colorCount; ++srcColorIndex)
            {
                const Palette::Color srcColor      = pl2.basePalette.colors[srcColorIndex];
                const int            invBlendRatio = 255 - alphaBlendRatio;
                const Palette::Color blendOuput{
                    uint8_t((invBlendRatio * srcColor.r + alphaBlendRatio * dstColor.r) / 0xFF),
                    uint8_t((invBlendRatio * srcColor.g + alphaBlendRatio * dstColor.g) / 0xFF),
                    uint8_t((invBlendRatio * srcColor.b + alphaBlendRatio * dstColor.b) / 0xFF),
                };
                pl2.alphaBlend[alphaBlendLevel][srcColorIndex].indices[dstColorIndex] =
                    pl2.basePalette.GetClosestColorIndex(blendOuput);
            }
        }
    }
}

void PL2CreateAdditiveBlend(PL2& pl2)
{
    for (size_t dstColorIndex = 0; dstColorIndex < Palette::colorCount; ++dstColorIndex)
    {
        const Palette::Color dstColor = pl2.basePalette.colors[dstColorIndex];
        for (size_t srcColorIndex = 0; srcColorIndex < Palette::colorCount; ++srcColorIndex)
        {
            const Palette::Color srcColor = pl2.basePalette.colors[srcColorIndex];
            const Palette::Color blendOuput{
                uint8_t(std::min(srcColor.r + dstColor.r, 0xFF)),
                uint8_t(std::min(srcColor.g + dstColor.g, 0xFF)),
                uint8_t(std::min(srcColor.b + dstColor.b, 0xFF)),
            };
            pl2.additiveBlend[srcColorIndex].indices[dstColorIndex] =
                pl2.basePalette.GetClosestColorIndex(blendOuput);
        }
    }
}

void PL2CreateMultiplicativeBlend(PL2& pl2)
{
    for (size_t dstColorIndex = 0; dstColorIndex < Palette::colorCount; ++dstColorIndex)
    {
        const Palette::Color dstColor = pl2.basePalette.colors[dstColorIndex];
        for (size_t srcColorIndex = 0; srcColorIndex < Palette::colorCount; ++srcColorIndex)
        {
            const Palette::Color srcColor = pl2.basePalette.colors[srcColorIndex];
            const Palette::Color blendOuput{
                uint8_t((srcColor.r * dstColor.r) / 0xFF),
                uint8_t((srcColor.g * dstColor.g) / 0xFF),
                uint8_t((srcColor.b * dstColor.b) / 0xFF),
            };
            pl2.multiplicativeBlend[srcColorIndex].indices[dstColorIndex] =
                pl2.basePalette.GetClosestColorIndex(blendOuput);
        }
    }
}

void PL2CreateColorshifts(PL2& pl2, ColorHSL hslColors[Palette::colorCount])
{
    for (int hueShiftIndex = 0; hueShiftIndex < 24; ++hueShiftIndex)
    {
        // Create a palette with hue variations
        // 24 hue variations of 15degrees
        for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
        {
            ColorHSL tmpColorHSL = hslColors[colorIndex];
            tmpColorHSL.hue += (double)hueShiftIndex * 15.0;
            if (tmpColorHSL.hue > 360.0) // Fix the hue range if needed
                tmpColorHSL.hue -= 360.0;

            pl2.hueVariations[hueShiftIndex].indices[colorIndex] =
                pl2.basePalette.GetClosestColorIndex(ConvertHSLtoRGB(tmpColorHSL));
        }
    }
    for (int hueShiftIndex = 0; hueShiftIndex < 24; ++hueShiftIndex)
    {
        for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
        {

            ColorHSL tmpColorHSL = hslColors[colorIndex];
            tmpColorHSL.hue += (double)hueShiftIndex * 15.0;
            if (tmpColorHSL.hue > 360.0) tmpColorHSL.hue -= 360.0;
            tmpColorHSL.sat = 0.5;
#ifdef USE_FIXED_VERSION
            tmpColorHSL.lum -= 0.1;
#else
            tmpColorHSL.lum -= double(0.1f); // The game actually uses the float constant which can
                                             // give slightly off results
#endif
            if (tmpColorHSL.lum < 0.0) tmpColorHSL.lum = 0.0;

            pl2.hueVariations[24 + hueShiftIndex].indices[colorIndex] =
                pl2.basePalette.GetClosestColorIndex(ConvertHSLtoRGB(tmpColorHSL));
        }
    }
    for (int hueShiftIndex = 0; hueShiftIndex < 24; ++hueShiftIndex)
    {
        for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
        {

            ColorHSL tmpColorHSL = hslColors[colorIndex];

            tmpColorHSL.hue += (double)hueShiftIndex * 15.0;
            if (tmpColorHSL.hue > 360.0) tmpColorHSL.hue -= 360.0;

            tmpColorHSL.sat = 0.5;

#ifdef USE_FIXED_VERSION
            tmpColorHSL.lum += 0.2;
#else
            tmpColorHSL.lum += double(0.2f); // The game actually uses the float constant which can
                                             // give slightly off results
#endif
            if (tmpColorHSL.lum > 1.0) { tmpColorHSL.lum = 1.0; }

            pl2.hueVariations[48 + hueShiftIndex].indices[colorIndex] =
                pl2.basePalette.GetClosestColorIndex(ConvertHSLtoRGB(tmpColorHSL));
        }
    }

    for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
    {
        ColorHSL tmpColorHSL = hslColors[colorIndex];
        tmpColorHSL.sat      = 0;
        tmpColorHSL.lum      = tmpColorHSL.lum / 2.0;
        pl2.hueVariations[72].indices[colorIndex] =
            pl2.basePalette.GetClosestColorIndex(ConvertHSLtoRGB(tmpColorHSL));
    }

    for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
    {
        ColorHSL tmpColorHSL = hslColors[colorIndex];
        tmpColorHSL.sat      = 0;

#ifdef USE_FIXED_VERSION
        tmpColorHSL.lum += 0.2;
        tmpColorHSL.lum /= 1.2;
#else
        // The game actually uses the float constant which can give slightly off results
        tmpColorHSL.lum += double(0.2f);
        tmpColorHSL.lum /= double(1.2f);
#endif
        pl2.hueVariations[73].indices[colorIndex] =
            pl2.basePalette.GetClosestColorIndex(ConvertHSLtoRGB(tmpColorHSL));
    }
    // See previous comment about reusing the previous colors
    for (int hueShiftIndex = 0; hueShiftIndex < 24; ++hueShiftIndex)
    {
        for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
        {
            ColorHSL tmpColorHSL = hslColors[colorIndex];
            // Only shift non red/orange (blood/fire?) colors
            if (tmpColorHSL.hue > 45.0 && tmpColorHSL.hue < 315.0)
            {

                tmpColorHSL.hue += (double)hueShiftIndex * 15.0;
                if (tmpColorHSL.hue > 360.0) tmpColorHSL.hue -= 360.0;

                pl2.hueVariations[74 + hueShiftIndex].indices[colorIndex] =
                    pl2.basePalette.GetClosestColorIndex(ConvertHSLtoRGB(tmpColorHSL));
            }
            else
            {
#ifdef USE_FIXED_VERSION
                pl2.hueVariations[74 + hueShiftIndex].indices[colorIndex] = colorIndex;
#else
                pl2.hueVariations[74 + hueShiftIndex].indices[colorIndex] =
                    pl2.hueVariations[73 + hueShiftIndex].indices[colorIndex];
#endif
            }
        }
    }

    for (int hueShiftIndex = 0; hueShiftIndex < 12; ++hueShiftIndex)
    {
        for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
        {

            ColorHSL tmpColorHSL = hslColors[colorIndex];
            tmpColorHSL.hue      = (double)hueShiftIndex * 30.0;
            tmpColorHSL.sat      = 1.0;
            pl2.hueVariations[99 + hueShiftIndex].indices[colorIndex] =
                pl2.basePalette.GetClosestColorIndex(ConvertHSLtoRGB(tmpColorHSL));
        }
    }

    for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
    {
        Palette::Color color = pl2.basePalette.colors[colorIndex];
        const double   colorMagnitudeDouble =
            std::sqrt((double)(color.r * color.r + color.g * color.g + color.b * color.b));
// Note : This does not make sense to not clamp the value, as the integer will wrap.
// However, this is what is done for the original game, so keep it here for reference
#ifdef USE_FIXED_VERSION
        // The one that clamps the value
        const uint8_t colorMagnitude = uint8_t(std::min(int(colorMagnitudeDouble), 255));
#else
        // The one from the game (d2cmp.dll), incorrect
        const uint8_t colorMagnitude = uint8_t(colorMagnitudeDouble);
#endif

        pl2.redTones.indices[colorIndex] =
            pl2.basePalette.GetClosestColorIndex({colorMagnitude, 0, 0});
        pl2.greenTones.indices[colorIndex] =
            pl2.basePalette.GetClosestColorIndex({0, colorMagnitude, 0});
        pl2.blueTones.indices[colorIndex] =
            pl2.basePalette.GetClosestColorIndex({0, 0, colorMagnitude});
    }
}

void PL2CreateMaxComponentBlend(PL2& pl2)
{
    for (size_t dstColorIndex = 0; dstColorIndex < Palette::colorCount; ++dstColorIndex)
    {
        const Palette::Color dstColor        = pl2.basePalette.colors[dstColorIndex];
        const uint8_t        maxComponentDst = std::max({dstColor.r, dstColor.g, dstColor.b});
        for (size_t srcColorIndex = 0; srcColorIndex < Palette::colorCount; ++srcColorIndex)
        {
            const uint8_t        invMaxComponentDst = 0xFF - maxComponentDst;
            const Palette::Color srcColor           = pl2.basePalette.colors[srcColorIndex];
            const Palette::Color blendOuput{
                uint8_t((invMaxComponentDst * srcColor.r + maxComponentDst * dstColor.r) / 0xFF),
                uint8_t((invMaxComponentDst * srcColor.g + maxComponentDst * dstColor.g) / 0xFF),
                uint8_t((invMaxComponentDst * srcColor.b + maxComponentDst * dstColor.b) / 0xFF),
            };
            pl2.maxComponentBlend[srcColorIndex].indices[dstColorIndex] =
                pl2.basePalette.GetClosestColorIndex(blendOuput);
        }
    }
}

void PL2CreateDarkenedUnitShift(PL2& pl2)
{
    for (size_t colorIndex = 0; colorIndex < Palette::colorCount; ++colorIndex)
    {
        Palette::Color tmpColor = pl2.basePalette.colors[colorIndex];
        tmpColor.r -= tmpColor.r / 3;
        tmpColor.g -= tmpColor.g / 3;
        tmpColor.b -= tmpColor.b / 3;

        pl2.darkenedColorShift.indices[colorIndex] = pl2.basePalette.GetClosestColorIndex(tmpColor);
    }
}

// clang-format off
static const Palette::Color24Bits defaultTextColors[13] = {
    {0xFF, 0xFF, 0xFF},
    {0xFF, 0x4D, 0x4D},
    {0x00, 0xFF, 0x00},
    {0x69, 0x69, 0xFF},
    {0xC7, 0xB3, 0x77},
    {0x69, 0x69, 0x69},
    {0x00, 0x00, 0x00},
    {0xD0, 0xC2, 0x7D},
    {0xFF, 0xA8, 0x00},
    {0xFF, 0xFF, 0x64},
    {0x00, 0x80, 0x00},
    {0xAE, 0x00, 0xFF},
    {0x00, 0xC8, 0x00},
};
// clang-format on

void PL2CreateTextColorshifts(PL2& pl2)
{
    static_assert(sizeof(defaultTextColors) == 13 * 3, "There must be 13 default text colors.");
    memcpy(pl2.textColors, defaultTextColors, sizeof(defaultTextColors));
    // It seems the game sets the first color as black. Again, weird, but this is what is done in
    // d2cmp.dll and seen in the game.
    pl2.textColorShifts[0].indices.fill(0u);
    for (size_t textColorIndex = 1; textColorIndex < 13; textColorIndex++)
    {
        const Palette::Color24Bits textColor = pl2.textColors[textColorIndex];
        for (size_t colorIndex = 0; colorIndex < 256; ++colorIndex)
        {
            const Palette::Color baseColor = pl2.basePalette.colors[colorIndex];
            // Note : We multiply the text color by the red component, I find this is a bit weird
            // but will keep it this way for reference, this is how the game (d2cmp.dll) does it.
            const uint8_t        textColorIntensity = baseColor.r;
            const Palette::Color newColor{
                uint8_t((textColor.r * textColorIntensity) / 0xFF),
                uint8_t((textColor.g * textColorIntensity) / 0xFF),
                uint8_t((textColor.b * textColorIntensity) / 0xFF),
            };
            pl2.textColorShifts[textColorIndex].indices[colorIndex] =
                pl2.basePalette.GetClosestColorIndex(newColor);
        }
    }
}
} // anonymous namespace

std::unique_ptr<PL2> PL2::CreateFromPalette(const Palette& palette)
{
    // Note : make_unique means the palshifts will be initialized to 0
    auto pl2Ptr = std::make_unique<PL2>();

    PL2& pl2        = *pl2Ptr;
    pl2.basePalette = palette;
    ColorHSL hslColors[Palette::colorCount];

    for (size_t i = 0; i < Palette::colorCount; i++)
    {
        hslColors[i] = ConvertRGBtoHSL(palette.colors[i]);
    }

    PL2CreateLightLevelVariations(pl2);
    PL2CreateInvColorVariations(pl2);
    PL2CreateSelectedUnitShift(pl2, hslColors);
    PL2CreateAlphaBlend(pl2);
    PL2CreateAdditiveBlend(pl2);
    PL2CreateMultiplicativeBlend(pl2);
    PL2CreateColorshifts(pl2, hslColors);
    PL2CreateMaxComponentBlend(pl2);
    PL2CreateDarkenedUnitShift(pl2);
    PL2CreateTextColorshifts(pl2);
    return pl2Ptr;
}

std::unique_ptr<PL2> PL2::ReadFromStream(IStream* stream)
{
    static_assert(sizeof(PL2) == 443175, "PL2 struct does not match the size of the files");
    static_assert(std::is_trivially_copyable<PL2>::value, "PL2 is not trivially copyable");
    if (stream && stream->good())
    {
        auto pl2 = std::make_unique<PL2>();
        if (stream->read(pl2.get(), sizeof(PL2)) == sizeof(PL2)) return pl2;
    }
    return nullptr;
}

} // namespace WorldStone
