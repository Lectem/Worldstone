#include "DrawSprite.h"
#include <Platform.h>
#include <algorithm>
#include <assert.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <limits>
#include "bgfxUtils.h"

static bool screenSpaceIsTopDown = true;

namespace
{

struct PosColorTexcoordVertex
{
    float    m_pos[3];
    uint32_t m_abgr;
    float    m_u;
    float    m_v;

    static void init()
    {
        ms_decl.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    }

    static bgfx::VertexDecl ms_decl;
};

WS_PRAGMA_DIAGNOSTIC_PUSH()
WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wglobal-constructors")
bgfx::VertexDecl PosColorTexcoordVertex::ms_decl;
WS_PRAGMA_DIAGNOSTIC_POP()

/*
      y
      ^ 0---2
      | | / |
      | 1---3
      +-----> x
*/

// Assume little-endian
constexpr uint32_t abgrBlack = 0xFF000000;
constexpr uint32_t abgrWhite = 0xFFFFFFFF;
constexpr uint32_t abgrRed   = 0x000000FF;
constexpr uint32_t abgrGreen = 0x0000FF00;
constexpr uint32_t abgrBlue  = 0xFFFF0000;

static const uint16_t s_quadTriList[] = {
    0, 1, 2, // 0
    1, 3, 2,
};

static const uint16_t s_quadTriStrip[] = {
    0, 1, 2, 3,
};
} // namespace

struct FrameRenderData
{
    bgfx::TextureHandle      spriteTexture = BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle vertexBuffer  = BGFX_INVALID_HANDLE;
};

struct SpriteRendererData
{
    bgfx::IndexBufferHandle m_quadIndexBuf = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle     m_program      = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle     m_texColor     = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle     m_palColor     = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle     m_paletteColor = BGFX_INVALID_HANDLE;
    int64_t                 m_timeOffset;
};

static bgfx::VertexBufferHandle
createVertexBufferFromSpriteFrame(const SpriteRenderer::Frame& frame)
{
    const bgfx::Memory* quadMemory = bgfx::alloc(4 * sizeof(PosColorTexcoordVertex));
    // (void*) to shut -Wcast-align since we know bgfx will provide 4 bytes alignment
    // Memory is actually stored right after the Memory struct, so at most 8 bytes alignment for
    // 32bits and 16bytes for 64bit
    PosColorTexcoordVertex* quadPtr = (PosColorTexcoordVertex*)(void*)quadMemory->data;

    // Since we need to cover the whole pixel for it to render, no need to -1
    const float lastColumn = float(frame.width);
    const float lastRow    = float(frame.height);
    // Negate as we render bottom up and not top down
    const float fOffsetX         = float(frame.offsetX);
    const float fOffsetY         = float(frame.offsetY);
    const float lastColumnOffset = fOffsetX + lastColumn;
    const float lastRowOffset    = fOffsetY + lastRow;
    const bool  topDown          = true;
    if (topDown == screenSpaceIsTopDown) {
        quadPtr[0] = {{fOffsetX, fOffsetY, 0.f}, abgrBlack, 0.f, 0.f};
        quadPtr[1] = {{fOffsetX, lastRowOffset, 0.f}, abgrGreen, 0.f, lastRow};
        quadPtr[2] = {{lastColumnOffset, fOffsetY, 0.f}, abgrRed, lastColumn, 0.f};
        quadPtr[3] = {{lastColumnOffset, lastRowOffset, 0.f}, abgrWhite, lastColumn, lastRow};
    }
    else
    {
        quadPtr[0] = {{fOffsetX, lastRowOffset, 0.f}, abgrGreen, 0.f, 0.f};
        quadPtr[1] = {{fOffsetX, fOffsetY, 0.f}, abgrBlack, 0.f, lastRow};
        quadPtr[2] = {{lastColumnOffset, lastRowOffset, 0.f}, abgrWhite, lastColumn, 0.f};
        quadPtr[3] = {{lastColumnOffset, fOffsetY, 0.f}, abgrRed, lastColumn, lastRow};
    }

    // Create static vertex buffer.
    return bgfx::createVertexBuffer(quadMemory, PosColorTexcoordVertex::ms_decl);
}

static bgfx::TextureHandle createTextureFromSprite(const SpriteRenderer::Frame& frame)
{
    bgfx::TextureHandle spriteTexture = BGFX_INVALID_HANDLE;
    uint32_t            flagsSprite   = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT;
    spriteTexture                     = bgfx::createTexture2D(
        frame.width, frame.height, false, 1, bgfx::TextureFormat::R8U, flagsSprite,
        bgfx::copy(frame.data, frame.width * frame.height * sizeof(uint8_t)));
    if (bgfx::isValid(spriteTexture)) bgfx::setName(spriteTexture, "SpriteTexture");
    return spriteTexture;
}

static FrameRenderData createFrameRenderData(const SpriteRenderer::Frame& frame)
{
    FrameRenderData renderData;
    renderData.spriteTexture = createTextureFromSprite(frame);
    renderData.vertexBuffer  = createVertexBufferFromSpriteFrame(frame);
    return renderData;
}
static void destroy(FrameRenderData& frameRenderData)
{
    bgfx::destroy(frameRenderData.vertexBuffer);
    bgfx::destroy(frameRenderData.spriteTexture);
}

void SpriteRenderer::SpriteRenderData::addSpriteFrame(const Frame& frame)
{
    framesData.push_back(createFrameRenderData(frame));
}
SpriteRenderer::SpriteRenderData::~SpriteRenderData()
{
    for (FrameRenderData& frameRenderData : framesData)
        destroy(frameRenderData);
}

void SpriteRenderer::init(const WorldStone::Palette& palette)
{
    data = std::make_unique<SpriteRendererData>();

    BX_UNUSED(s_quadTriList, s_quadTriStrip);
    BX_UNUSED(abgrBlack, abgrWhite, abgrRed, abgrGreen, abgrBlue);
    // Create vertex stream declaration.
    PosColorTexcoordVertex::init();

    // Create static index buffer.
    data->m_quadIndexBuf = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_quadTriStrip, sizeof(s_quadTriStrip)));

    // Create the samplers
    data->m_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
    data->m_palColor = bgfx::createUniform("s_palColor", bgfx::UniformType::Int1);

    setPalette(palette);

    // Create program from shaders.
    data->m_program    = loadProgram("vs_sprite", "fs_sprite");
    data->m_timeOffset = bx::getHPCounter();
}

int SpriteRenderer::shutdown()
{
    spritesToRender.clear();
    spritesData.clear();
    // Cleanup.
    bgfx::destroy(data->m_paletteColor);
    bgfx::destroy(data->m_palColor);
    bgfx::destroy(data->m_texColor);
    bgfx::destroy(data->m_program);
    bgfx::destroy(data->m_quadIndexBuf);

    return 0;
}

void SpriteRenderer::setPalette(const WorldStone::Palette& palette)
{
    if (bgfx::isValid(data->m_paletteColor)) bgfx::destroy(data->m_paletteColor);

    using Palette = WorldStone::Palette;
    static_assert(sizeof(Palette::Color24Bits) == 3, "");
    auto paletteRGB888 = bgfx::alloc(sizeof(Palette::Color24Bits) * Palette::colorCount);
    for (size_t i = 0; i < Palette::colorCount; i++)
    {
        const WorldStone::Palette::Color color                      = palette.colors[i];
        ((WorldStone::Palette::Color24Bits*)paletteRGB888->data)[i] = {color.r, color.g, color.b};
    }
    data->m_paletteColor =
        bgfx::createTexture2D(Palette::colorCount, 1, false, 1, bgfx::TextureFormat::RGB8,
                              BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT, paletteRGB888);
    bgfx::setName(data->m_paletteColor, "SpritePalette");
}

SpriteRenderer::SpriteRenderDataHandle SpriteRenderer::createSpriteRenderData()
{
    spritesData.push_back(std::make_shared<SpriteRenderData>());
    return spritesData.back();
}

void SpriteRenderer::destroySpriteRenderData(SpriteRenderDataHandle renderDataHandle)
{
    SpriteRenderData* renderDataPtr = renderDataHandle.lock().get();
    assert(renderDataPtr != nullptr);
    for (auto& ptr : spritesData)
    {
        if (ptr.get() == renderDataPtr) {
            assert(ptr.unique());
            ptr = nullptr;
            return;
        }
    }
}

void SpriteRenderer::recycleSpritesData()
{
    // This is really aggressive but right now we ask the user to always check/recreate the data
    // when needed All the handle system has to be written (and more than anything, we have to get
    // rid of shared_ptr)
    if (spritesData.size() > 20) {
        spritesData.erase(std::remove_if(spritesData.begin(), spritesData.end(),
                                         [](const std::shared_ptr<SpriteRenderData>& ptr) {
                                             return ptr == nullptr || ptr.unique();
                                         }),
                          spritesData.end());
    }
}

void SpriteRenderer::pushDrawRequest(SpriteRenderDataHandle spriteHandle,
                                     const DrawRequest&     drawRequest)
{
    auto spritePtr = spriteHandle.lock();
    assert(spritePtr != nullptr);
    spritesToRender.emplace_back(std::make_pair(std::move(spritePtr), drawRequest));
}

void SpriteRenderer::drawFrame(const FrameRenderData& renderData)
{
    // Set vertex and index buffer.
    bgfx::setVertexBuffer(0, renderData.vertexBuffer);
    bgfx::setIndexBuffer(data->m_quadIndexBuf);

    // Bind texture
    bgfx::setTexture(0, data->m_texColor, renderData.spriteTexture);
    bgfx::setTexture(1, data->m_palColor, data->m_paletteColor);

    // Set render states.
    // No ZBuffer usage for now, it is 2d... Perhaps it'll be used for layers later ?
    bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW
                   | BGFX_STATE_PT_TRISTRIP);

    // Submit primitive for rendering to view 0.
    bgfx::submit(0, data->m_program);
}

bool SpriteRenderer::draw(int screenWidth, int screenHeight)
{
    float at[3]  = {0.f, 0.f, 0.f};
    float eye[3] = {0.f, 0.f, -35.0f};

    // Set view and projection matrix for view 0.
    float view[16];
    bx::mtxLookAtRh(view, eye, at);

    float proj[16];
    if (screenSpaceIsTopDown) {

        // RightHanded origin at bottom left, z coming from the screen
        //
        //       _ +Z
        //       /|
        //      /
        //     /
        //    +----------> +X
        //    |
        //    |
        //    |
        //    |
        //   \/
        //   +Y
        //
        //
        bx::mtxOrtho(proj, 0.f, float(screenWidth), float(screenHeight), 0.f, -1000.f, 1000.f, 0.f,
                     bgfx::getCaps()->homogeneousDepth);
    }
    else
    {

        // RightHanded origin at bottom left, z coming from the screen
        //
        //     +Y
        //     ^
        //     |
        //     |
        //     |
        //     |
        //     +----------> +X
        //    /
        //   /
        // +Z
        bx::mtxOrtho(proj, 0.f, float(screenWidth), 0.f, float(screenHeight), 1000.f, -1000.f, 0.f,
                     bgfx::getCaps()->homogeneousDepth);
    }

    bgfx::setViewTransform(0, nullptr, proj);

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16_t(screenWidth), uint16_t(screenHeight));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    // i is just a debug, will disappear later
    static size_t i = size_t(-1);
    for (const auto& spriteData : spritesToRender)
    {
        i++;
        i %= (spriteData.first->framesData.size() * 10);
        const DrawRequest&     drawRequest = spriteData.second;
        const FrameRenderData& renderData  = spriteData.first->framesData[drawRequest.frame];
        const float            scale       = drawRequest.scale;

        // Submit 1 quad
        float mtx[16];
        bx::mtxIdentity(mtx);
        bx::mtxScale(mtx, scale);
        // Translation
        const auto& translation = drawRequest.translation;
        mtx[12]                 = translation.x;
        mtx[13]                 = translation.y;
        mtx[14]                 = 0.f;

        bgfx::setTransform(&mtx);

        drawFrame(renderData);
    }

    spritesToRender.clear();
    recycleSpritesData();
    return true;
}

SpriteRenderer::SpriteRenderer() {}
SpriteRenderer::~SpriteRenderer() {}
