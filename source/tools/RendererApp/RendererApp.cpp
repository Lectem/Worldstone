#include "RendererApp.h"
#include <FileStream.h>
#include <bgfx/bgfx.h>
#include <dcc.h>
#include "DrawSprite.h"
#include "imgui/imgui_bgfx.h"

bool RendererApp::initAppThread()
{
    if (!BaseApp::initAppThread()) return false;

    imguiCreate(imguiAllocator);
    using WorldStone::DCC;
    DCC testDCC;
    testDCC.initDecoder(std::make_unique<WorldStone::FileStream>("sprites/CRHDBRVDTHTH.dcc"));
    DCC::Direction                           firstDir;
    WorldStone::SimpleImageProvider<uint8_t> imageprovider;
    testDCC.readDirection(firstDir, 0, imageprovider);
    const auto&         firstFrame = firstDir.frameHeaders[0];
    WorldStone::Palette pal;
    pal.decode("palettes/pal.dat");
    assert(pal.isValid());
    static_assert(sizeof(WorldStone::Palette::Color) == 3, "");
    DrawSprite::init({uint16_t(firstFrame.width), uint16_t(firstFrame.height),
                      imageprovider.getImage(0).buffer, (const uint8_t*)pal.colors.data()});
    return true;
}

void RendererApp::shutdownAppThread()
{
    DrawSprite::shutdown();
    imguiDestroy();
    BaseApp::shutdownAppThread();
}

void RendererApp::executeAppLoopOnce()
{
    // Input processing comes here
    const Inputs inputs = inputsManager.receiveAndProcessEvents();

    float wheelVerticalScroll = 0.f;
    for (const SDL_Event& event : inputs.events)
    {
        switch (event.type)
        {
        case SDL_QUIT: requireExit(); break;
        case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_F1) {
                showBgfxStats = !showBgfxStats;
            }
            break;
        case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0)
                wheelVerticalScroll = 1;
            else if (event.wheel.y < 0)
                wheelVerticalScroll = -1;
#if SDL_VERSION_ATLEAST(2, 0, 4)
            if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) wheelVerticalScroll *= -1.f;
#endif
            break;
        }
    }

    // Logic update comes here

    // Imgui
    const Inputs::MouseState& mouseState = inputs.mouseState;
    imguiBeginFrame(
        int32_t(mouseState.x), int32_t(mouseState.y),
        0 | (mouseState.buttonsMask & SDL_BUTTON(SDL_BUTTON_LEFT) ? IMGUI_MBUT_LEFT : 0)
            | (mouseState.buttonsMask & SDL_BUTTON(SDL_BUTTON_RIGHT) ? IMGUI_MBUT_RIGHT : 0)
            | (mouseState.buttonsMask & SDL_BUTTON(SDL_BUTTON_MIDDLE) ? IMGUI_MBUT_MIDDLE : 0),
        wheelVerticalScroll, uint16_t(windowWidth), uint16_t(windowHeight));

    ImGui::ShowDemoWindow();

    {
        ImGui::Begin("Debug");
        ImGui::Checkbox("Display bgfx stats", &showBgfxStats);
        bgfx::setDebug(BGFX_DEBUG_TEXT | (showBgfxStats ? BGFX_DEBUG_STATS : 0));
        ImGui::End();
    }

    imguiEndFrame();

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16_t(windowWidth), uint16_t(windowHeight));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    DrawSprite::draw(windowWidth, windowHeight);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    // This will also wait for the render thread to finish presenting the frame
    bgfx::frame();
}
