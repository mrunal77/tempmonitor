#pragma once

#include <cstddef>
#include <string>

namespace tempmonitor {

std::string format_temperature(double celsius);

std::string format_sensor_count(std::size_t count);

} // namespace tempmonitor
