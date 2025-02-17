#include "ui.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include "app.hpp"

Imgui_UIManagerDeleter UIManagerDeleter;

std::unique_ptr<Imgui_UIManager, Imgui_UIManagerDeleter> UIManager(new Imgui_UIManager(), UIManagerDeleter);

SDL_AppResult Imgui_UIManager::initUI() {
    // TODO: Add Error checking

    SDL_Log("Initializing ImGui");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Set DPI scale to match the window
    auto displays = SDL_GetDisplays(NULL);
    auto dpi      = SDL_GetDisplayContentScale(displays[0]);
    io.FontGlobalScale = dpi;

    ImGui_ImplSDL3_InitForOpenGL(AppState->window.get(), AppState->glContext.get());
    ImGui_ImplOpenGL3_Init();

    SDL_Log("Initialized ImGui");

    return SDL_APP_CONTINUE;
}

void Imgui_UIManager::processEvent(SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void Imgui_UIManager::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);

    for (auto& uiFunction : uiFunctions) {
        uiFunction.second();
    }
}

void Imgui_UIManager::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Imgui_UIManager::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

unsigned int Imgui_UIManager::addUIFunction(UIFunction uiFunction) {
    int id = uiFunctions.size();
    uiFunctions[id] = uiFunction;
    return id;
}

void Imgui_UIManager::removeUIFunction(unsigned int id) {
    uiFunctions.erase(id);
}