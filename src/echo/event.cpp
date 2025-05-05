#include "cinder.hpp"

#include "echo/event.hpp"

namespace echo {

void EventManager::init() {
    cinder::log("Event handler initialized.");
}

SDL_AppResult EventManager::handle(SDL_Event* event) {
    if (!m_eventHandlers.contains(event->type))
        return SDL_APP_CONTINUE;

    auto handlers = m_eventHandlers[event->type];
    SDL_AppResult result = SDL_APP_CONTINUE;
    for (auto handler : handlers) {
        SDL_AppResult subResult = handler(event);
        if (subResult > result)
            result = subResult;
    }
    return result;
}

void EventManager::add(Uint32 eventType, EventHandlerFunction* handler) {
    if (!m_eventHandlers.contains(eventType)) {
        m_eventHandlers[eventType] = std::vector<EventHandlerFunction*>();
    }

    m_eventHandlers[eventType].push_back(handler);
}

void EventManager::cancel(Uint32 eventType, EventHandlerFunction* handler) {
    if (!m_eventHandlers.contains(eventType))
        return;

    auto handlers = m_eventHandlers[eventType];
    for (auto it = handlers.begin(); it != handlers.end(); it++) {
        if (*it == handler) {
            handlers.erase(it);
            return;
        }
    }
}

EventManager::~EventManager() {
    // The handlers are not owned by the EventHandler
    // so we don't delete them here
    cinder::log("Event handler destroyed.");
}

};