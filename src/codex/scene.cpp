#include "codex/scene.hpp"

#include "imgui.h"

#include <string>
#include <array>

namespace Codex {

const std::array<std::string, TEXTURE_TYPE_COUNT> TextureTypeStrings = {
    "textureDiffuse",
    "textureNormal",
    "textureAORoughnessMetallic"
};

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