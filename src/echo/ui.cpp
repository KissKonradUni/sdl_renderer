#include "cinder.hpp"
#include "echo/ui.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <IconsMaterialSymbols.h>

namespace echo {

SDL_AppResult UIManager::init(SDL_Window* window, SDL_GLContextState* glContext) {
    // TODO: Add Error checking
    cinder::log("Initializing UIManager...");

    cinder::log("Initializing ImGui...");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Set DPI scale to match the window
    auto displays = SDL_GetDisplays(NULL);
    auto dpi      = SDL_GetDisplayContentScale(displays[0]);
    SDL_free(displays);
    //io.FontGlobalScale = dpi;

    // Load custom font
    auto fontSize = 16.0f * dpi;
    io.Fonts->AddFontFromFileTTF("assets/fonts/FiraCode-Regular.ttf", fontSize);

    // Load icons
    auto iconSize = fontSize * 1.25f;
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconSize;
    iconsConfig.GlyphOffset = ImVec2(0, 0.2f * iconSize);
    static const ImWchar iconsRanges[] = { ICON_MIN_MS, ICON_MAX_MS, 0 };
    io.Fonts->AddFontFromFileTTF("assets/fonts/" FONT_ICON_FILE_NAME_MSR, iconSize, &iconsConfig, iconsRanges);

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init();

    cinder::log("Initialized ImGui.");

    cinder::log("Initialized UIManager.");

    return SDL_APP_CONTINUE;
}

void UIManager::processEvent(SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void UIManager::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);

    for (auto& uiFunction : m_uiFunctions) {
        uiFunction.second();
    }
}

void UIManager::render() {
    ImGui::Render();
    auto data = ImGui::GetDrawData();
    if (data) {
        ImGui_ImplOpenGL3_RenderDrawData(data);
    }
}

void UIManager::cleanup() {
    m_uiFunctions.clear();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

unsigned int UIManager::addUIFunction(UIFunction uiFunction) {
    int id = m_uiFunctions.size();
    m_uiFunctions[id] = uiFunction;
    return id;
}

void UIManager::removeUIFunction(unsigned int id) {
    m_uiFunctions.erase(id);
}

UIManager::~UIManager() {
    this->cleanup();
    cinder::log("ImGui closed.");
    cinder::log("UIManager destroyed.");
}

void UIManager::openAssetBrowserDialog(
    int filter,
    dialogSuccessCallback successCallback,
    dialogCancelCallback cancelCallback,
    bool dontFilter
) {
    m_lastFilter = filter;
    m_lastSuccessCallback = successCallback;
    m_lastCancelCallback = cancelCallback;

    if (filter == 0) {
        m_lastFilter = codex::FileType::TEXT_FILE;
    }

    if (dontFilter) {
        m_lastFilter = 0;
    }

    m_lastDialogID = this->addUIFunction(UIManager::assetBrowserDialog);
}

void UIManager::assetBrowserDialog() {
    auto instance = cinder::app->getUIManager();

    ImGui::SetNextWindowBgAlpha(1.0f);
    if (ImGui::Begin("Asset Picker...", nullptr, 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove
    )) {
        auto screenSize = ImGui::GetMainViewport()->Size;
        
        ImGui::SetWindowSize(screenSize, ImGuiCond_Always);
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);

        using namespace cinder;
        auto library = app->getLibrary();

        library->assetsList([](codex::FileNode* node) {
            auto instance = cinder::app->getUIManager();
            instance->m_lastSuccessCallback(node);
            instance->removeUIFunction(instance->m_lastDialogID);
        }, 0.95f);

        if (ImGui::Button("Cancel")) {
            if (instance->m_lastCancelCallback) {
                instance->m_lastCancelCallback();
            }
            instance->removeUIFunction(instance->m_lastDialogID);
        }

        ImGui::End();
    }
}

}; // namespace echo