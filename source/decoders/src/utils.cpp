//
// Created by Lectem on 05/11/2016.
//
#include "utils.h"
#include <stdio.h>
#include <fmt/format.h>
#include "palette.h"

namespace WorldStone
{
namespace Utils
{

void exportToPGM(const char* output, const uint8_t* data, int width, int height, int maxVal)
{
    assert(width > 0 && height > 0);
    if (maxVal == -1) {
        for (int i = 0; i < width * height; ++i)
        {
            if (maxVal < data[i]) maxVal = data[i];
        }
    }
    int   bpp  = maxVal >= 256 ? 2 : 1;
    FILE* file = fopen(output, "wb");
    if (file) {
        fmt::print(file, "P5 {} {} {}\n", width, height, maxVal);
        fwrite(data, static_cast<size_t>(bpp), static_cast<size_t>(width * height), file);
        fclose(file);
    }
}

void exportToPPM(const char* output, const uint8_t* data, int width, int height,
                 const Palette& palette)
{
    assert(width > 0 && height > 0);
    assert(palette.isValid());
    FILE* file = fopen(output, "wb");
    if (file) {
        fmt::print(file, "P6 {} {} 255\n", width, height);
        for (size_t i = 0; i < static_cast<size_t>(width * height); ++i)
        {
            const std::vector<Palette::Color>& colors = palette.colors;
            fputc(colors[data[i]].r, file);
            fputc(colors[data[i]].g, file);
            fputc(colors[data[i]].b, file);
        }
        fclose(file);
    }
}
} // namespace Utils
} // namespace WorldStone
