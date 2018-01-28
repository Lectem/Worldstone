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

    ImGuiIO& io                    = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab]        = SDLK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown]   = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home]       = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End]        = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert]     = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete]     = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]      = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape]     = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = SDLK_a;
    io.KeyMap[ImGuiKey_C]          = SDLK_c;
    io.KeyMap[ImGuiKey_V]          = SDLK_v;
    io.KeyMap[ImGuiKey_X]          = SDLK_x;
    io.KeyMap[ImGuiKey_Y]          = SDLK_y;
    io.KeyMap[ImGuiKey_Z]          = SDLK_z;

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

    // Ideally we would not set the imgui IO directly here as we should have a
    // wrapper that parses SDL events
    ImGuiIO& imguiIO = ImGui::GetIO();
    for (const SDL_Event& event : inputs.events)
    {
        switch (event.type)
        {
        case SDL_QUIT: requireExit(); break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_F1) {
                showBgfxStats = !showBgfxStats;
            }
            // FALLTHROUGH
        case SDL_KEYUP:
        {
            int key               = event.key.keysym.sym & ~SDLK_SCANCODE_MASK;
            imguiIO.KeysDown[key] = (event.type == SDL_KEYDOWN);
            imguiIO.KeyShift      = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            imguiIO.KeyCtrl       = ((SDL_GetModState() & KMOD_CTRL) != 0);
            imguiIO.KeyAlt        = ((SDL_GetModState() & KMOD_ALT) != 0);
            imguiIO.KeySuper      = ((SDL_GetModState() & KMOD_GUI) != 0);
            break;
        }
        case SDL_MOUSEWHEEL: { float mouseWheelY = float(event.wheel.y);
#if SDL_VERSION_ATLEAST(2, 0, 4)
            if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) mouseWheelY *= -1.f;
#endif
            // Accumulate wheel if we have multiple events in one frame
            imguiIO.MouseWheel += mouseWheelY;
            break;
        }
        case SDL_TEXTINPUT: imguiIO.AddInputCharactersUTF8(event.text.text); break;
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
        uint16_t(windowWidth), uint16_t(windowHeight));

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
