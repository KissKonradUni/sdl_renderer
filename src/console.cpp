#include "console.hpp"

#include <imgui.h>
#include <SDL3/SDL.h>

std::unique_ptr<Imgui_Console> console = std::make_unique<Imgui_Console>();

void Imgui_Console::log(const std::string& message) {
    newMessage(Message{ 
        message, MSG_INFO, SDL_GetTicks(), 0
    });
}

void Imgui_Console::warn(const std::string& message) {
    newMessage(Message{ 
        message, MSG_WARN, SDL_GetTicks(), 0
    });
}

void Imgui_Console::error(const std::string& message) {
    newMessage(Message{ 
        message, MSG_ERROR, SDL_GetTicks(), 0
    });
}

void Imgui_Console::drawConsole() {
    ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoBackground);

    ImGui::BeginChild("ConsoleOptions", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, false);
        if (ImGui::Button("Clear")) {
            messages.clear();
        }
        ImGui::SameLine();    
        ImGui::Checkbox("Scroll to bottom", &scrollToBottom);
    ImGui::EndChild();

    ImGui::Separator();

    ImGui::BeginChild("ConsoleMessages", ImVec2(0, -36), false, false);
    for (const auto& message : messages) {
        switch (message.level) {
            case MSG_INFO:
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "[%8ld]%s %s", message.timestamp, this->prefix.c_str(), message.message.c_str());
                break;
            case MSG_WARN:
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[%8ld]%s %s", message.timestamp, this->prefix.c_str(), message.message.c_str());
                break;
            case MSG_ERROR:
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[%8ld]%s %s", message.timestamp, this->prefix.c_str(), message.message.c_str());
                break;
        }
    }
    if (scrollToBottom)
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    
    ImGui::BeginChild("ConsoleInput", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, false);
        ImGui::PushItemWidth(-96);
        auto result = ImGui::InputText("##ConsoleInput", inputBuffer.data(), inputBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (ImGui::Button("Send", ImVec2(90, 0)) || result) {
            log(inputBuffer.data());
            inputBuffer[0] = '\0';
        }
    ImGui::EndChild();

    ImGui::End();
}

void Imgui_Console::newMessage(const Message& message) {
    messages.push_back(message);
}

unsigned int Imgui_Console::getMessageWidth(const std::string& message) {
    return ImGui::CalcTextSize(message.c_str()).x;
}