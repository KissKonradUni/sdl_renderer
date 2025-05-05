#pragma once

#include "app.hpp"

#include <memory>

namespace cinder {

extern std::unique_ptr<App> app;

void log(const std::string& message);
void warn(const std::string& message);
void error(const std::string& message);

}