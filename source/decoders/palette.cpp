//
// Created by Lectem on 06/11/2016.
//

#include "palette.h"
#include <stdio.h>

void Palette::Decode(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (file) {
        colors.resize(256);
        for (int i = 0; i < 256; i++)
        {
            // order is BGR, not RGB
            colors[i].b = (uint8_t)fgetc(file);
            colors[i].g = (uint8_t)fgetc(file);
            colors[i].r = (uint8_t)fgetc(file);
        }
        fclose(file);
        _isValid = true;
    }
}
