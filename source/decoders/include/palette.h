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
        uint8_t r, g, b;
    };

    static const int colorCount = 256;
    std::array<Color, colorCount> colors;

    void decode(const char* filename);
    void decode(IStream* file);
    bool isValid() const { return valid; }

private:
    bool valid = false;
};
} // namespace WorldStone
