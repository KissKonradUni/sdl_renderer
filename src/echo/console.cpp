#include "echo/console.hpp"

#include <imgui.h>
#include <SDL3/SDL.h>

#define ANSI_RED "\x1b[31m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_RESET "\x1b[0m"

namespace Echo {

MessageTimestamp::MessageTimestamp(unsigned long milliseconds) {
    this->milliseconds = milliseconds % 1000;
    this->seconds      = (milliseconds / 1000) % 60;
    this->minutes      = (milliseconds / 60000) % 60;
    this->hours        = (milliseconds / 3600000) % 24;
}

void Console::log(const std::string& message) {
    newMessage(Message{ 
        message, MSG_INFO, MessageTimestamp(SDL_GetTicks())
    });
}

void Console::warn(const std::string& message) {
    newMessage(Message{ 
        message, MSG_WARN, MessageTimestamp(SDL_GetTicks())
    });
}

void Console::error(const std::string& message) {
    newMessage(Message{ 
        message, MSG_ERROR, MessageTimestamp(SDL_GetTicks())
    });
}

Console::~Console() {
    m_messages.clear();
}

void Console::drawConsole() {
    ImGui::Begin("Console", nullptr);

    ImGui::BeginChild("ConsoleOptions", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, false);
        if (ImGui::Button("Clear")) {
            m_messages.clear();
        }
        ImGui::SameLine();    
        ImGui::Checkbox("Scroll to bottom", &m_scrollToBottom);
    ImGui::EndChild();

    ImGui::Separator();

    ImGui::BeginChild("Consolem_messages", ImVec2(0, -46), false, false);
    ImVec4 color;
    for (const auto& message : m_messages) {
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
            this->m_prefix.c_str(),
            message.message.c_str()
        );

    }

    if (m_scrollToBottom)
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    
    ImGui::BeginChild("ConsoleInput", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, false);
        ImGui::PushItemWidth(-96);
        auto result = ImGui::InputText("##ConsoleInput", m_inputBuffer.data(), m_inputBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (ImGui::Button("Send", ImVec2(90, 0)) || result) {
            log(std::string("Command issued: ") + m_inputBuffer.data());
            if (std::string_view(m_inputBuffer.data()).starts_with("exit"))
                SDL_PushEvent(new SDL_Event{SDL_EVENT_QUIT});
            m_inputBuffer[0] = '\0';
            ImGui::SetKeyboardFocusHere(-1);
        }
    ImGui::EndChild();

    ImGui::End();
}

void Console::newMessage(const Message& message) {
    m_messages.push_back(message);

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
        this->m_prefix.c_str(),
        message.message.c_str(),
        ANSI_RESET
    );
}

unsigned int Console::getMessageWidth(const std::string& message) {
    return ImGui::CalcTextSize(message.c_str()).x;
}

void log(const std::string& message) {
    Console::instance().log(message);
}

void warn(const std::string& message) {
    Console::instance().warn(message);
}

void error(const std::string& message) {
    Console::instance().error(message);
}
 
void consoleWindow() {
    Console::instance().drawConsole();
}

} // namespace Echo