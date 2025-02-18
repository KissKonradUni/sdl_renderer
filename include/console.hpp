#pragma once

#include <string>
#include <array>
#include <list>

#include <SDL3/SDL.h>

namespace Echo {

enum MessageLevel {
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
};

class Console {
public:
    static Console& instance() {
        static Console instance;
        return instance;
    }

    void log(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    void drawConsole();

protected:
    Console() = default;
    ~Console();

    std::string prefix = "Cinder";
    std::list<Message> messages = {};
    
    bool scrollToBottom = true;
    std::array<char, 256> inputBuffer = {};

    void newMessage(const Message& message);

    unsigned int getMessageWidth(const std::string& message);
};

class ModuleConsole {
public:
    ModuleConsole(const std::string& module);
    ~ModuleConsole();

    void log(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
protected:
    std::string prefix;
};

void log(const std::string& message);
void warn(const std::string& message);
void error(const std::string& message);

void drawConsole();

}; // namespace Echo