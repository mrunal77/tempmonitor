#include "test_framework.hpp"
#include "ui/formatting.hpp"

void run_formatting_tests() {
    CHECK(tempmonitor::format_temperature(45.0) == "45.0 °C");
    CHECK(tempmonitor::format_temperature(-5.5) == "-5.5 °C");
    CHECK(tempmonitor::format_temperature(0.0) == "0.0 °C");
    CHECK(tempmonitor::format_temperature(42.06) == "42.1 °C");
    CHECK(tempmonitor::format_sensor_count(0) == "0 sensors");
    CHECK(tempmonitor::format_sensor_count(1) == "1 sensor");
    CHECK(tempmonitor::format_sensor_count(2) == "2 sensors");
}
