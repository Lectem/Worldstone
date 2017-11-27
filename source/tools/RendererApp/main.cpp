#include "main.h"
#include <SDL.h>
#include <fmt/format.h>


BaseApp::~BaseApp() { shutdown(); }

int main(int, char**)
{
    BaseApp app;
    app.run();
    return 0;
}
