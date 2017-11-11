//
// Created by Lectem on 06/11/2016.
//

#include "palette.h"
#include <stdio.h>
#include <FileStream.h>
namespace WorldStone
{

void Palette::decode(const char* filename)
{
    FileStream str{filename};
    decode(&str);
}

void Palette::decode(IStream* file)
{
    if (file && file->good()) {
        for (size_t i = 0; i < colorCount; i++)
        {
            // order is BGR, not RGB
            colors[i].b = static_cast<uint8_t>(file->getc());
            colors[i].g = static_cast<uint8_t>(file->getc());
            colors[i].r = static_cast<uint8_t>(file->getc());
        }
        valid = file->good();
    }
}
} // namespace WorldStone
