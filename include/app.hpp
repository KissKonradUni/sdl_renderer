#pragma once

#include <SDL3/SDL.h>

#include <memory>

class SDL_AppState {
public:
    std::shared_ptr<SDL_Window>       window;
    std::shared_ptr<SDL_GPUDevice> gpuDevice;
};

class SDL_AppDeleter {
public:
    void operator()(SDL_Window* window) {
        SDL_DestroyWindow(window);
        SDL_Log("Window destroyed");
    }
    void operator()(SDL_GPUDevice* gpuDevice) {
        SDL_DestroyGPUDevice(gpuDevice);
        SDL_Log("GPU device destroyed");
    }
    void operator()(SDL_AppState* appState) {
        SDL_ReleaseWindowFromGPUDevice(appState->gpuDevice.get(), appState->window.get());
        SDL_Log("Window released from GPU device");

        appState->gpuDevice.reset();
        appState->window.reset();
    }
};
extern SDL_AppDeleter AppDeleter;

extern std::unique_ptr<SDL_AppState, SDL_AppDeleter> AppState;