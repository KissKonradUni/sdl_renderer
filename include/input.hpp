#pragma once

#include <SDL3/SDL.h>

#include <map>
#include <vector>

namespace Echo {

typedef SDL_AppResult (EventHandlerFunction)(SDL_Event*);

class Events {
public:
    static Events& instance() {
        static Events instance;
        return instance;
    }

    /**
     * @brief Handle an incoming SDL event
     * @note If the event type is not found in the eventHandlers map, the event is ignored
     * 
     * @param event The SDL event to handle
     */
    SDL_AppResult handleEvent(SDL_Event* event);
    static SDL_AppResult handle(SDL_Event* event);

    /**
     * @brief Add an event handler to the eventHandlers map
     * @note The caller is responsible for managing the memory of the handler,
     *       as the EventHandler does not take ownership of the handler.
     *       The handler should be removed from the eventHandlers map before it is deleted.
     * 
     * @param eventType UInt32 representing the type of SDL event to handle
     * @param handler Pointer to the function to handle the event
     */
    void addEvent(Uint32 eventType, EventHandlerFunction* handler);
    static void add(Uint32 eventType, EventHandlerFunction* handler);

    /**
     * @brief Remove an event handler from the eventHandlers map
     * @note You must pass the same pointer to the handler that was passed to the add method
     *       in order to remove the handler from the eventHandlers map
     * 
     * @param eventType UInt32 representing the type of SDL event to handle
     * @param handler Pointer to the function to remove from the eventHandlers map
     */
    void cancelEvent(Uint32 eventType, EventHandlerFunction* handler);
    static void cancel(Uint32 eventType, EventHandlerFunction* handler);
protected:
    std::map<Uint32, std::vector<EventHandlerFunction*>> eventHandlers;

    Events();
    ~Events();
};

}; // namespace Echo