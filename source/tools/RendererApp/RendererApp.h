#pragma once
#include <bx/allocator.h>
#include "BaseApp.h"
#include "DrawSprite.h"

class RendererApp : public BaseApp
{
public:
    RendererApp();
    ~RendererApp();

private:
    bool initAppThread() override;
    void shutdownAppThread() override;
    void executeAppLoopOnce() override;

    bool                 showBgfxStats = false;
    bx::DefaultAllocator imguiAllocator;
    std::unique_ptr<class FileBrowser> fileBrowser;
    SpriteRenderer                     spriteRenderer;
};
