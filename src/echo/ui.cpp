#include "app.hpp"
#include "echo/ui.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <IconsMaterialSymbols.h>

namespace echo {

SDL_AppResult UI::initUI() {
    // TODO: Add Error checking

    echo::log("Initializing ImGui...");

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

    ImGui_ImplSDL3_InitForOpenGL(cinder::App::getWindowPtr(), cinder::App::getGLContextPtr());
    ImGui_ImplOpenGL3_Init();

    echo::log("Initialized ImGui.");

    return SDL_APP_CONTINUE;
}

void UI::processEvent(SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void UI::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);

    for (auto& uiFunction : m_uiFunctions) {
        uiFunction.second();
    }
}

void UI::render() {
    ImGui::Render();
    auto data = ImGui::GetDrawData();
    if (data) {
        ImGui_ImplOpenGL3_RenderDrawData(data);
    }
}

void UI::cleanup() {
    m_uiFunctions.clear();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

unsigned int UI::addUIFunction(UIFunction uiFunction) {
    int id = m_uiFunctions.size();
    m_uiFunctions[id] = uiFunction;
    return id;
}

void UI::removeUIFunction(unsigned int id) {
    m_uiFunctions.erase(id);
}

UI::~UI() {
    this->cleanup();
    echo::log("ImGui closed.");
    echo::log("Echo UI destroyed.");
}

}; // namespace echo