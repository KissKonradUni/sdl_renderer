#pragma once

#include <string>
#include <array>
#include <list>

#include <SDL3/SDL.h>

namespace echo {

#ifndef ANSI_RESET
    #define ANSI_RED "\x1b[31m"
    #define ANSI_YELLOW "\x1b[33m"
    #define ANSI_RESET "\x1b[0m"
#endif // ANSI_COLORS

enum MessageLevel : uint8_t {
    MSG_INFO = 0,
    MSG_WARN = 1,
    MSG_ERROR = 2
};

struct MessageTimestamp {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t milliseconds;

    MessageTimestamp(unsigned long milliseconds);
};

struct Message {
    std::string message;
    MessageLevel level;
    MessageTimestamp timestamp;

    Message(std::string message, MessageLevel level, MessageTimestamp timestamp)
        : message(message), level(level), timestamp(timestamp) {}
    ~Message() = default;
};

class Console {
public:
    Console() = default;
    ~Console();

    void init();

    void log(const std::string& message) {
        newMessage(message, MSG_INFO);
    }
    
    template <typename... Args>
    void warn(const std::string& message) {
        newMessage(message, MSG_WARN);
    }

    template <typename... Args>
    void error(const std::string& message) {
        newMessage(message, MSG_ERROR);
    }

    void drawConsole();
protected:
    std::string m_prefix = "Cinder";
    std::list<Message*> m_messages = {};
    
    bool m_scrollToBottom = true;
    std::array<char, 256> m_inputBuffer = {};

    char m_outputBuffer[2048] = {};

    void newMessage(const std::string& text, MessageLevel level) {
        auto timestamp = MessageTimestamp(SDL_GetTicks());
        auto message = new Message(text, level, timestamp);
        m_messages.push_back(message);

        std::string color = ANSI_RESET;
        if (message->level == MSG_WARN) {
            color = ANSI_YELLOW;
        } else if (message->level == MSG_ERROR) {
            color = ANSI_RED;
        }

        SDL_Log(
            "%s[%0.2d:%0.2d:%0.2d:%0.3d][%s] %s%s",
            color.c_str(),
            message->timestamp.hours,
            message->timestamp.minutes,
            message->timestamp.seconds,
            message->timestamp.milliseconds,
            this->m_prefix.c_str(),
            message->message.c_str(),
            ANSI_RESET
        );
    }

    unsigned int getMessageWidth(const std::string& message);
};

}; // namespace echo