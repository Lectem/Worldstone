#pragma once

#include <stdint.h>
#include <Vector.h>
#include <memory>
#include <Palette.h>

class SpriteRenderer
{
public:
    struct Frame
    {
        int16_t        offsetX;
        int16_t        offsetY;
        uint16_t       width;
        uint16_t       height;
        const uint8_t* data;
    };

    class SpriteRenderData
    {
        friend class SpriteRenderer;

    public:
        void addSpriteFrame(const Frame& frame);
        ~SpriteRenderData();

    protected:
        WorldStone::Vector<struct FrameRenderData> framesData;
    };
    using SpriteRenderDataHandle = std::weak_ptr<SpriteRenderData>; // This is a poor-man's handle
                                                                    // while waiting for a real
                                                                    // handle system

    void init(const WorldStone::Palette& palette);
    int shutdown();
    void setPalette(const WorldStone::Palette& palette);

    SpriteRenderDataHandle createSpriteRenderData();
    void                   destroySpriteRenderData(SpriteRenderDataHandle);

    struct Pos2D
    {
        float x, y;
    };
    struct DrawRequest
    {
        Pos2D    translation;
        uint32_t frame;
        float    scale;
    };
    void pushDrawRequest(SpriteRenderDataHandle spriteHandle, const DrawRequest& translation);

    SpriteRenderer();
    ~SpriteRenderer();

    bool draw(int screenWidth, int screenHeight);

private:
    void drawFrame(const struct FrameRenderData& renderData);
    void recycleSpritesData();

    WorldStone::Vector<std::shared_ptr<SpriteRenderData>> spritesData;
    WorldStone::Vector<std::pair<std::shared_ptr<SpriteRenderData>, DrawRequest>> spritesToRender;

    // Should actually be pimpl, but for now data is enough
    std::unique_ptr<struct SpriteRendererData> data;
};
