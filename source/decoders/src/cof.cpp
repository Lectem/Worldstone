//
// Created by Lectem on 23/10/2016.
//

#include "cof.h"
#include <FileStream.h>
#include <SystemUtils.h>
#include <fmt/format.h>
#include "utils.h"

// TODO : Remove asserts and replace with proper error handling

namespace WorldStone
{

constexpr uint8_t     COF::componentsNumber;
constexpr const char* COF::componentsNames[COF::componentsNumber];

bool COF::read(const StreamPtr& streamPtr)
{
    if (streamPtr && streamPtr->good()) {
        static_assert(std::is_trivially_copyable<Header>(),
                      "COF::Header must be trivially copyable");
        static_assert(sizeof(Header) == 4 + 6 * sizeof(uint32_t),
                      "COF::Header struct needs to be packed");
        streamPtr->read(&header, sizeof(header));
        if (streamPtr->fail() || header.version != 20) return false;

        static_assert(std::is_trivially_copyable<Layer>(), "COF::Layer must be trivially copyable");
        static_assert(sizeof(Layer) == 9 * sizeof(uint8_t), "COF::Layer struct needs to be packed");
        layers.resize(header.layers);
        streamPtr->read(layers.data(), header.layers * sizeof(Layer));
        for (const Layer& layer : layers)
        {
            WS_UNUSED(layer);
            assert(layer.weaponClass[3] == '\0');
        }

        keyframes.resize(header.frames);
        streamPtr->read(keyframes.data(), header.frames);

        const size_t layersOrderSize = header.frames * header.directions * header.layers;
        layersOrder.resize(layersOrderSize);
        streamPtr->read(layersOrder.data(), layersOrderSize);

        return streamPtr->good();
    }
    return false;
}

} // namespace WorldStone
