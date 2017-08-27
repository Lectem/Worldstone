//
// Created by Lectem on 06/11/2016.
//

#include "palette.h"
#include <stdio.h>
#include <FileStream.h>
namespace WorldStone
{

void Palette::Decode(const char* filename)
{
    FileStream str{filename};
    Decode(&str);
}

void Palette::Decode(Stream* file)
{
    if (file && file->good()) {
        colors.resize(colorCount);
        for (size_t i = 0; i < colorCount; i++)
        {
            // order is BGR, not RGB
            colors[i].b = static_cast<uint8_t>(file->getc());
            colors[i].g = static_cast<uint8_t>(file->getc());
            colors[i].r = static_cast<uint8_t>(file->getc());
        }
        _isValid = file->good();
    }
}
} // namespace WorldStone
