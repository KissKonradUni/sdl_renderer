#include "app.hpp"

static SDL_AppDeleter AppDeleter;

static std::unique_ptr<SDL_AppState, SDL_AppDeleter> AppState(new SDL_AppState(), AppDeleter);