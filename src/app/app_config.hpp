#pragma once

#include <chrono>

namespace tempmonitor {

struct AppConfig {
    std::chrono::milliseconds refresh_interval{1000};
};

} // namespace tempmonitor
