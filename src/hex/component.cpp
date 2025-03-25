#include "hex/component.hpp"
#include "imgui.h"

namespace Hex {

void Component::editorUI() {
    ImGui::Text("%s", getPrettyName().c_str());
    ImGui::Separator();
}

}; // namespace Hex