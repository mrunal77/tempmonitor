#pragma once

#include "sensors/temperature_reading.hpp"

#include <filesystem>
#include <vector>

namespace tempmonitor {

class SensorReader {
public:
    explicit SensorReader(std::filesystem::path sysfs_root = "/sys");

    [[nodiscard]] std::vector<TemperatureReading> read() const;

private:
    std::filesystem::path sysfs_root_;
};

} // namespace tempmonitor
