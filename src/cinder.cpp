#include "cinder.hpp"

namespace cinder {

void log(const std::string& message) {
    if (app && app->getConsole())
        app->getConsole()->log(message);
}

void warn(const std::string& message) {
    if (app && app->getConsole())
        app->getConsole()->warn(message);
}

void error(const std::string& message) {
    if (app && app->getConsole())
        app->getConsole()->error(message);
}

}