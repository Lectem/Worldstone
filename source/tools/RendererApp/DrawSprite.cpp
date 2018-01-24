#include "DrawSprite.h"
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <limits>
#include "bgfxUtils.h"

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

BX_PRAGMA_DIAGNOSTIC_PUSH()
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wglobal-constructors")
bgfx::VertexDecl PosColorTexcoordVertex::ms_decl;
BX_PRAGMA_DIAGNOSTIC_POP()

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
// constexpr uint16_t            uv0              = 0;
// constexpr uint16_t            uv1              = std::numeric_limits<int16_t>::max();
constexpr float               uv0              = 0.f;
constexpr float               uv1              = 1.f;
static PosColorTexcoordVertex s_quadVertices[] = {
    {{0.0f, 1.0f, 0.0f}, abgrGreen, uv0, uv1},
    {{0.0f, 0.0f, 0.0f}, abgrBlack, uv0, uv0},
    {{1.0f, 1.0f, 0.0f}, abgrWhite, uv1, uv1},
    {{1.0f, 0.0f, 0.0f}, abgrRed, uv1, uv0},
};

static const uint16_t s_quadTriList[] = {
    0, 1, 2, // 0
    1, 3, 2,
};

static const uint16_t s_quadTriStrip[] = {
    0, 1, 2, 3,
};
} // namespace

namespace DrawSprite
{

static bgfx::VertexBufferHandle m_vbh          = BGFX_INVALID_HANDLE;
static bgfx::IndexBufferHandle  m_ibh          = BGFX_INVALID_HANDLE;
static bgfx::ProgramHandle      m_program      = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle      s_texColor     = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle      s_palColor     = BGFX_INVALID_HANDLE;
static bgfx::TextureHandle      m_textureColor = BGFX_INVALID_HANDLE;
static bgfx::TextureHandle      m_paletteColor = BGFX_INVALID_HANDLE;
static int64_t                  m_timeOffset;

void init(const Sprite& sprite)
{
    BX_UNUSED(s_quadTriList, s_quadTriStrip);
    BX_UNUSED(abgrBlack, abgrWhite, abgrRed, abgrGreen, abgrBlue);
    // Create vertex stream declaration.
    PosColorTexcoordVertex::init();

    // Create static vertex buffer.
    m_vbh = bgfx::createVertexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_quadVertices, sizeof(s_quadVertices)), PosColorTexcoordVertex::ms_decl);

    // Create static index buffer.
    m_ibh = bgfx::createIndexBuffer(
        // Static data can be passed with bgfx::makeRef
        bgfx::makeRef(s_quadTriStrip, sizeof(s_quadTriStrip)));

    s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
    s_palColor = bgfx::createUniform("s_palColor", bgfx::UniformType::Int1);

    uint32_t flagsSprite = BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT;
    m_textureColor       = bgfx::createTexture2D(
        sprite.width, sprite.height, false, 1, bgfx::TextureFormat::R8U, flagsSprite,
        bgfx::copy(sprite.data, sprite.width * sprite.height * sizeof(uint8_t)));
    bgfx::setName(m_textureColor, "SpriteTexture");

    m_paletteColor = bgfx::createTexture2D(256, 1, false, 1, bgfx::TextureFormat::RGB8, flagsSprite,
                                           bgfx::copy(sprite.palette, 256 * 3));
    bgfx::setName(m_paletteColor, "SpritePalette");

    float lastPixelColumn = float(sprite.width - 1);
    float lastPixelRow    = float(sprite.height - 1);
    bool  topDown         = true;
    if (topDown) {
        s_quadVertices[0] = {{0.0f, 1.0f, 0.0f}, abgrGreen, 0.f, 0.f};
        s_quadVertices[1] = {{0.0f, 0.0f, 0.0f}, abgrBlack, 0.f, lastPixelRow};
        s_quadVertices[2] = {{1.0f, 1.0f, 0.0f}, abgrWhite, lastPixelColumn, 0.f};
        s_quadVertices[3] = {{1.0f, 0.0f, 0.0f}, abgrRed, lastPixelColumn, lastPixelRow};
    }
    else
    {

        s_quadVertices[0] = {{0.0f, 1.0f, 0.0f}, abgrGreen, 0.f, lastPixelRow};
        s_quadVertices[1] = {{0.0f, 0.0f, 0.0f}, abgrBlack, 0.f, 0.f};
        s_quadVertices[2] = {{1.0f, 1.0f, 0.0f}, abgrWhite, lastPixelColumn, lastPixelRow};
        s_quadVertices[3] = {{1.0f, 0.0f, 0.0f}, abgrRed, lastPixelColumn, 0.f};
    }
    // int16_t midWidth = (sprite.width - 1) / 2;
    // int16_t midHeight = (sprite.height - 1) / 2;
    // s_quadVertices[0] = { { 0.0f, 1.0f, 0.0f }, abgrGreen,midWidth,midHeight};
    // s_quadVertices[1] = { { 0.0f, 0.0f, 0.0f }, abgrBlack,midWidth,midHeight};
    // s_quadVertices[2] = { { 1.0f, 1.0f, 0.0f }, abgrWhite,midWidth,midHeight};
    // s_quadVertices[3] = { { 1.0f, 0.0f, 0.0f }, abgrRed,  midWidth,midHeight};

    // Create program from shaders.
    m_program    = loadProgram("vs_sprite", "fs_sprite");
    m_timeOffset = bx::getHPCounter();
}

int shutdown()
{

    // Cleanup.
    bgfx::destroy(m_paletteColor);
    bgfx::destroy(s_palColor);
    bgfx::destroy(m_textureColor);
    bgfx::destroy(s_texColor);
    bgfx::destroy(m_program);
    bgfx::destroy(m_ibh);
    bgfx::destroy(m_vbh);

    return 0;
}

bool draw(int width, int height)
{
    //        float time = (float)((bx::getHPCounter() - m_timeOffset) /
    //        double(bx::getHPFrequency()));
    float at[3]  = {0.0f, 0.0f, 0.0f};
    float eye[3] = {0.0f, 0.0f, -35.0f};

    // Set view and projection matrix for view 0.
    float view[16];
    bx::mtxLookAtRh(view, eye, at);

    float proj[16];
    bx::mtxProjRh(proj, 60.0f, float(width) / float(height), 0.1f, 100.0f,
                  bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);

    // RightHanded
    bx::mtxOrtho(proj, 0.f, float(width), 0.f, float(height), 1000.0f, -1000.f, 0.0f,
                 bgfx::getCaps()->homogeneousDepth);
    // bx::mtxOrthoRh(proj, -1.f, 1.f, -1.f, 1.f, 1000.0f, -1.f, 0.0f,
    // bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, nullptr, proj);

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    // Submit 1 quad
    float mtx[16];
    bx::mtxIdentity(mtx);
    bx::mtxScale(mtx, 100);
    // Translation
    mtx[12] = 0.f;
    mtx[13] = 0.f;
    mtx[14] = 0.f;

    // Set model matrix for rendering.
    bgfx::setTransform(mtx);

    // Set vertex and index buffer.
    bgfx::setVertexBuffer(0, m_vbh);
    bgfx::setIndexBuffer(m_ibh);

    // Bind texture
    bgfx::setTexture(0, s_texColor, m_textureColor);
    bgfx::setTexture(1, s_palColor, m_paletteColor);

    // Set render states.
    bgfx::setState(0 | BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP);

    // Submit primitive for rendering to view 0.
    bgfx::submit(0, m_program);
    return true;
}
}
