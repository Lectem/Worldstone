#include "main.h"
#include <SDL.h>
#include <fmt/format.h>

class BaseApp
{
private:
    bool        stopRunning = false;
    SDL_Window* mainWindow  = nullptr;

    int init()
    {
        stopRunning = false;

        // Setup SDL
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            fmt::print("Error: {}\n", SDL_GetError());
            return -1;
        }

        mainWindow =
            SDL_CreateWindow("Main window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280,
                             720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (!mainWindow) {
            fmt::print("Error: {}\n", SDL_GetError());
            return -1;
        }
        return 0;
    }

    void shutdown()
    {
        SDL_DestroyWindow(mainWindow);
        mainWindow = nullptr;

        SDL_Quit();
    }

protected:
    virtual void executeOneLoop()
    {

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) requireExit();
        }

        SDL_Delay(10); // Don't burn our CPU
    }

    void requireExit() { stopRunning = true; }

public:
    BaseApp() { init(); }
    ~BaseApp() { shutdown(); }

    void run()
    {
        while (!stopRunning)
        {
            executeOneLoop();
        }
    }
};

int main(int, char**)
{
    BaseApp app;
    app.run();
    return 0;
}