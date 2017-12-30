#pragma once
#include <bx/allocator.h>
#include "BaseApp.h"

class RendererApp : public BaseApp
{
    bool initAppThread() override;
    void shutdownAppThread() override;
    void executeAppLoopOnce() override;

    bool                 showBgfxStats = false;
    bx::DefaultAllocator imguiAllocator;
};
