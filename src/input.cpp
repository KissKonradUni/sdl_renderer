#include "input.hpp"

std::unique_ptr<SDL_EventHandler> EventHandler = std::make_unique<SDL_EventHandler>();

SDL_AppResult SDL_EventHandler::handle(SDL_Event* event) {
    if (!eventHandlers.contains(event->type))
        return SDL_APP_CONTINUE;

    auto handlers = eventHandlers[event->type];
    SDL_AppResult result = SDL_APP_CONTINUE;
    for (auto handler : handlers) {
        SDL_AppResult subResult = handler(event);
        if (subResult > result)
            result = subResult;
    }
    return result;
}

void SDL_EventHandler::add(Uint32 eventType, EventHandlerFunction* handler) {
    if (!eventHandlers.contains(eventType)) {
        eventHandlers[eventType] = std::vector<EventHandlerFunction*>();
    }

    eventHandlers[eventType].push_back(handler);
}

void SDL_EventHandler::remove(Uint32 eventType, EventHandlerFunction* handler) {
    if (!eventHandlers.contains(eventType))
        return;

    auto handlers = eventHandlers[eventType];
    handlers.erase(std::remove(handlers.begin(), handlers.end(), handler), handlers.end());
}

SDL_EventHandler::SDL_EventHandler() {
    eventHandlers = std::map<Uint32, std::vector<EventHandlerFunction*>>();
}

SDL_EventHandler::~SDL_EventHandler() {
    // No need for custom destructor
    // The handlers are not owned by the EventHandler
    SDL_Log("Event handler destroyed");
}