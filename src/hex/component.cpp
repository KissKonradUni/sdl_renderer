#include "hex/component.hpp"
#include "imgui.h"

namespace hex {

void Component::editorUI() {
    ImGui::Text("%s", getPrettyName().c_str());
    ImGui::Separator();
}

}; // namespace hex