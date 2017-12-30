#pragma once

#include <SDL_events.h>
#include <condition_variable>
#include <mutex>
#include <vector>

struct Inputs
{
    struct MouseState
    {
        uint32_t buttonsMask;
        int      x;
        int      y;
    } mouseState;

    std::vector<SDL_Event> events;

    void reset() { events.clear(); }
};

class InputsManager
{
    std::mutex              inputMutex;
    std::condition_variable conditionVariable;
    Inputs                  inputs;
    bool                    transferNotReceived = false;

public:
    friend struct ScopedTransfer;
    class ScopedTransfer
    {
        InputsManager& inputsManager;
        ScopedTransfer(const ScopedTransfer&) = delete;

    public:
        ScopedTransfer(InputsManager& manager) : inputsManager(manager)
        {
            std::unique_lock<std::mutex> lock(inputsManager.inputMutex);
            inputsManager.inputs.reset();
        }
        ~ScopedTransfer()
        {
            inputsManager.transferNotReceived = true;
            inputsManager.conditionVariable.notify_all();
        }

        void PushMouseState(Inputs::MouseState mouseState)
        {
            inputsManager.inputs.mouseState = mouseState;
        }

        void PushEvent(SDL_Event event) { inputsManager.inputs.events.push_back(event); }
    };

    Inputs receiveAndProcessEvents()
    {
        std::unique_lock<std::mutex> lock(inputMutex);
        conditionVariable.wait(lock, [this]() { return transferNotReceived; });
        transferNotReceived = false;
        return inputs;
    }
};
