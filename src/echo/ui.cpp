#include "echo/ui.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include "app.hpp"

namespace Echo {

SDL_AppResult UI::initUI() {
    // TODO: Add Error checking

    Echo::log("Initializing ImGui...");

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
    io.Fonts->AddFontFromFileTTF("assets/fonts/FiraCode-Regular.ttf", 16.0f * dpi);
    m_smallFont = io.Fonts->AddFontFromFileTTF("assets/fonts/FiraCode-Regular.ttf", 10.0f * dpi);

    ImGui_ImplSDL3_InitForOpenGL(Cinder::App::getWindowPtr(), Cinder::App::getGLContextPtr());
    ImGui_ImplOpenGL3_Init();

    Echo::log("Initialized ImGui.");

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
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

void UI::pushSmallFont() {
    ImGui::PushFont(m_smallFont);
}

UI::~UI() {
    this->cleanup();
    Echo::log("ImGui closed.");
    Echo::log("Echo UI destroyed.");
}

}; // namespace Echo