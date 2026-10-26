#pragma once

#include <string>

namespace tempmonitor {

struct TemperatureReading {
    std::string id;
    std::string label;
    std::string source;
    double celsius = 0.0;
};

} // namespace tempmonitor
