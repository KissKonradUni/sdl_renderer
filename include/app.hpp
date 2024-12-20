#pragma once

#include <SDL3/SDL.h>

#include <memory>

class SDL_AppDeleter {
public:
    void operator()(SDL_Window* window) {
        SDL_DestroyWindow(window);
    }
    void operator()(SDL_GPUDevice* gpuDevice) {
        SDL_DestroyGPUDevice(gpuDevice);
    }
};
extern SDL_AppDeleter AppDeleter;

typedef class SDL_AppState {
public:
    std::shared_ptr<SDL_Window>       window;
    std::shared_ptr<SDL_GPUDevice> gpuDevice;
} SDL_AppState;
extern std::unique_ptr<SDL_AppState> AppState;