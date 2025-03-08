#include "codex/scene.hpp"

#include "imgui.h"

namespace Codex {

void Scene::draw() {
    
}

void Scene::sceneExplorerWindow() {
    ImGui::Begin("Scene");

    //auto windowSize = ImGui::GetWindowContentRegionMax();

    ImGui::End();
}

void Scene::inspectorWindow() {
    ImGui::Begin("Inspector");

    ImGui::Text("No object selected.");

    ImGui::End();
}

}; // namespace Codex