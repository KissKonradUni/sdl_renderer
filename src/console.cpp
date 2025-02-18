#include "console.hpp"

#include <imgui.h>
#include <SDL3/SDL.h>

#define ANSI_RED "\x1b[31m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_RESET "\x1b[0m"

std::unique_ptr<Imgui_Console> console = std::make_unique<Imgui_Console>();

MessageTimestamp::MessageTimestamp(unsigned long milliseconds) {
    this->milliseconds = milliseconds % 1000;
    this->seconds      = (milliseconds / 1000) % 60;
    this->minutes      = (milliseconds / 60000) % 60;
    this->hours        = (milliseconds / 3600000) % 24;
}

void Imgui_Console::log(const std::string& message) {
    newMessage(Message{ 
        message, MSG_INFO, MessageTimestamp(SDL_GetTicks())
    });
}

void Imgui_Console::warn(const std::string& message) {
    newMessage(Message{ 
        message, MSG_WARN, MessageTimestamp(SDL_GetTicks())
    });
}

void Imgui_Console::error(const std::string& message) {
    newMessage(Message{ 
        message, MSG_ERROR, MessageTimestamp(SDL_GetTicks())
    });
}

Imgui_Console::~Imgui_Console() {
    messages.clear();
    SDL_Log("[*] Console destroyed");
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
    ImVec4 color;
    for (const auto& message : messages) {
        switch (message.level) {
            case MSG_INFO:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                break;
            case MSG_WARN:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case MSG_ERROR:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
        }
        ImGui::TextColored(
            color,
            "[%0.2d:%0.2d:%0.2d:%0.3d][%s] %s",
            message.timestamp.hours,
            message.timestamp.minutes,
            message.timestamp.seconds,
            message.timestamp.milliseconds,
            this->prefix.c_str(),
            message.message.c_str()
        );

    }

    if (scrollToBottom)
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    
    ImGui::BeginChild("ConsoleInput", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, false);
        ImGui::PushItemWidth(-96);
        auto result = ImGui::InputText("##ConsoleInput", inputBuffer.data(), inputBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (ImGui::Button("Send", ImVec2(90, 0)) || result) {
            log(std::string("Command issued: ") + inputBuffer.data());
            if (std::string_view(inputBuffer.data()).starts_with("exit"))
                SDL_PushEvent(new SDL_Event{SDL_EVENT_QUIT});
            inputBuffer[0] = '\0';
            ImGui::SetKeyboardFocusHere(-1);
        }
    ImGui::EndChild();

    ImGui::End();
}

void Imgui_Console::newMessage(const Message& message) {
    messages.push_back(message);

    std::string color = ANSI_RESET;
    if (message.level == MSG_WARN) {
        color = ANSI_YELLOW;
    } else if (message.level == MSG_ERROR) {
        color = ANSI_RED;
    }

    SDL_Log(
        "%s[%0.2d:%0.2d:%0.2d:%0.3d][%s] %s%s",
        color.c_str(),
        message.timestamp.hours,
        message.timestamp.minutes,
        message.timestamp.seconds,
        message.timestamp.milliseconds,
        this->prefix.c_str(),
        message.message.c_str(),
        ANSI_RESET
    );
}

unsigned int Imgui_Console::getMessageWidth(const std::string& message) {
    return ImGui::CalcTextSize(message.c_str()).x;
}