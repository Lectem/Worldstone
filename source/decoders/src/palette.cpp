//
// Created by Lectem on 06/11/2016.
//

#include "palette.h"
#include <stdio.h>

void Palette::Decode(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (file) {
        colors.resize(colorCount);
        for (size_t i = 0; i < colorCount; i++)
        {
            // order is BGR, not RGB
            colors[i].b = static_cast<uint8_t>(fgetc(file));
            colors[i].g = static_cast<uint8_t>(fgetc(file));
            colors[i].r = static_cast<uint8_t>(fgetc(file));
        }
        fclose(file);
        _isValid = true;
    }
}
