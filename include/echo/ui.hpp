#pragma once

#include "codex/filenode.hpp"

#include <SDL3/SDL.h>
#include <map>

typedef void (*UIFunction)();

namespace echo {

class UIManager {
public:
    UIManager() = default;
    ~UIManager();

    SDL_AppResult init(SDL_Window* window, SDL_GLContextState* glContext);
    void processEvent(SDL_Event* event);
    void newFrame();
    void render();
    void cleanup();

    unsigned int addUIFunction(UIFunction uiFunction);
    void removeUIFunction(unsigned int id);

    using dialogSuccessCallback = void (*)(codex::FileNode* node);
    using dialogCancelCallback = void (*)();
    void openAssetBrowserDialog(
        int filter,
        dialogSuccessCallback successCallback,
        dialogCancelCallback cancelCallback = nullptr,
        bool dontFilter = false
    );
protected:
    std::map<unsigned int, UIFunction> m_uiFunctions;

    static void assetBrowserDialog();

    unsigned int m_lastDialogID = 0;
    uint8_t m_lastFilter = codex::FileType::TEXT_FILE;
    dialogSuccessCallback m_lastSuccessCallback = nullptr;
    dialogCancelCallback  m_lastCancelCallback = nullptr;
};

}; // namespace echo