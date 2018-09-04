#include "RendererApp.h"
#include <FileStream.h>
#include <MpqArchive.h>
#include <bgfx/bgfx.h>
#include <dcc.h>
#include "DrawSprite.h"
#include "FileBrowser.h"
#include "imgui/imgui_bgfx.h"

RendererApp::RendererApp()  = default;
RendererApp::~RendererApp() = default;

bool RendererApp::initAppThread()
{
    if (!BaseApp::initAppThread()) return false;

    imguiCreate(imguiAllocator);

    ImGuiIO& io                    = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab]        = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown]   = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home]       = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End]        = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert]     = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete]     = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space]      = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter]      = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape]     = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C]          = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V]          = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X]          = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y]          = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z]          = SDL_SCANCODE_Z;

    io.ConfigFlags = 0 | ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

    io.SetClipboardTextFn = [](void*, const char* text) { SDL_SetClipboardText(text); };
    io.GetClipboardTextFn = [](void*) -> const char* { return SDL_GetClipboardText(); };

    io.IniFilename = "imgui.ini";
    io.LogFilename = nullptr; // "imgui_log.txt";

    WorldStone::Palette pal;
    if (!pal.decode("palettes/pal.dat")) return false;
    spriteRenderer.init(pal);

    fileBrowser = std::make_unique<FileBrowser>();
    return true;
}

void RendererApp::shutdownAppThread()
{
    fileBrowser.reset(nullptr);
    spriteRenderer.shutdown();
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
            WS_FALLTHROUGH;
        case SDL_KEYUP:
        {
            int key = event.key.keysym.scancode;
            IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(imguiIO.KeysDown));
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

    fileBrowser->display(spriteRenderer);

    imguiEndFrame();

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16_t(windowWidth), uint16_t(windowHeight));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    spriteRenderer.draw(windowWidth, windowHeight);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    // This will also wait for the render thread to finish presenting the frame
    bgfx::frame();
}
