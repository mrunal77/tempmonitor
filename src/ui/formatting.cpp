#include "ui/formatting.hpp"

#include <iomanip>
#include <locale>
#include <sstream>

namespace tempmonitor {

std::string format_temperature(double celsius) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::fixed << std::setprecision(1) << celsius << " °C";
    return stream.str();
}

std::string format_sensor_count(std::size_t count) {
    return std::to_string(count) + (count == 1 ? " sensor" : " sensors");
}

} // namespace tempmonitor
