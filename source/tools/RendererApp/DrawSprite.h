#pragma once

#include <stdint.h>
namespace DrawSprite
{

struct Sprite
{
    uint16_t       width;
    uint16_t       height;
    const uint8_t* data;
    const uint8_t* palette;
};
void init(const Sprite& sprite);

int shutdown();

bool draw(int width, int height);
}
