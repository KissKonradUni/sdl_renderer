#include "echo/input.hpp"
#include "echo/console.hpp"

namespace Echo {

SDL_AppResult Events::handleEvent(SDL_Event* event) {
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
SDL_AppResult Events::handle(SDL_Event* event) {
    return Events::instance().handleEvent(event);
}

void Events::addEvent(Uint32 eventType, EventHandlerFunction* handler) {
    if (!m_eventHandlers.contains(eventType)) {
        m_eventHandlers[eventType] = std::vector<EventHandlerFunction*>();
    }

    m_eventHandlers[eventType].push_back(handler);
}
void Events::add(Uint32 eventType, EventHandlerFunction* handler) {
    Events::instance().addEvent(eventType, handler);
}

void Events::cancelEvent(Uint32 eventType, EventHandlerFunction* handler) {
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
void Events::cancel(Uint32 eventType, EventHandlerFunction* handler) {
    Events::instance().cancelEvent(eventType, handler);
}

Events::Events() {
    m_eventHandlers = std::map<Uint32, std::vector<EventHandlerFunction*>>();
}

Events::~Events() {
    // No need for custom destructor
    // The handlers are not owned by the EventHandler
    Echo::log("Event handler destroyed.");
}

};