#pragma once
#include <SDL.h>
#include <atomic>

#include "Inputs.h"

class BaseApp
{
private:
    std::atomic_bool stopRunning = ATOMIC_VAR_INIT(false);
    SDL_Window*      mainWindow  = nullptr;

    int init();

    void shutdown();

    void executeLoopOnce();

protected:
    InputsManager inputsManager;

    int windowWidth  = 1280;
    int windowHeight = 720;

    virtual bool initAppThread();
    virtual void shutdownAppThread();
    // You are expected to call inputsManager.receiveAndProcessEvents() and bgfx::frame() in your
    // loop
    virtual void executeAppLoopOnce();
    void         requireExit() { stopRunning = true; }

public:
    BaseApp() { init(); }
    virtual ~BaseApp() { shutdown(); }

    void run();
    void runAppThread();
};
