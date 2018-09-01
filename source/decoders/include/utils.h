//
// Created by Lectem on 05/11/2016.
//

#pragma once

#include <stdint.h>
#include "ImageView.h"
#include "Palette.h"

namespace WorldStone
{
namespace Utils
{
/**
 * Export some 8bit grayscale data to the PGM format
 * @param output The name of the ouptut file
 * @param data   The raw data of the image
 * @param width  The width of the image
 * @param height The height of the image
 * @param maxVal The maximum grayscale value, if >= 256 it means 2 bytes per pixel (might have
 * endianness issues)
 *               If the value is -1, use the highest value of data and assume 1 byte per pixel
 */
void exportToPGM(const char* output, const uint8_t* data, int width, int height, int maxVal = -1);
/// @overload void exportToPGM(const char*, ImageView<const uint8_t>, int)
void exportToPGM(const char* output, ImageView<const uint8_t> image, int maxVal = -1);

/**
 * Export a paletted image to the PPM format
 * @param output  The name of the ouptut file
 * @param data    The raw data (palette indices)
 * @param width   The width of the image
 * @param height  The height of the image
 * @param palette The palette to pick colors from
 */
void exportToPPM(const char* output, const uint8_t* data, int width, int height,
                 const Palette& palette);
} // namespace Utils
} // namespace WorldStone
