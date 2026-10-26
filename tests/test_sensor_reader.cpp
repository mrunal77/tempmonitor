#include "test_framework.hpp"

#include "sensors/sensor_reader.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

using tempmonitor::SensorReader;
using tempmonitor::TemperatureReading;

class FakeSysfs {
public:
    FakeSysfs() : root_(make_root()) {
        std::filesystem::create_directories(root_);
    }

    ~FakeSysfs() {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    const std::filesystem::path& root() const {
        return root_;
    }

    void write(const std::filesystem::path& relative_path, std::string_view contents) const {
        const auto path = root_ / relative_path;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary);
        file << contents;
    }

private:
    static std::filesystem::path make_root() {
        const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
        return std::filesystem::temp_directory_path() /
               ("tempmonitor-test-" + std::to_string(suffix));
    }

    std::filesystem::path root_;
};

const TemperatureReading* find_reading(const std::vector<TemperatureReading>& readings,
                                       std::string_view id) {
    for (const auto& reading : readings) {
        if (reading.id == id) {
            return &reading;
        }
    }
    return nullptr;
}

void test_hwmon_and_thermal_discovery() {
    FakeSysfs sysfs;
    sysfs.write("class/hwmon/hwmon0/name", "coretemp\n");
    sysfs.write("class/hwmon/hwmon0/temp1_input", "45000\n");
    sysfs.write("class/hwmon/hwmon0/temp1_label", "Package id 0\n");

    sysfs.write("class/hwmon/hwmon1/name", "nvme\n");
    sysfs.write("class/hwmon/hwmon1/temp1_input", "38000\n");
    sysfs.write("class/hwmon/hwmon1/temp2_input", "not-a-number\n");

    sysfs.write("class/hwmon/hwmon2/name", "k10temp\n");
    sysfs.write("class/hwmon/hwmon2/temp1_input", "  -5500 \n");
    sysfs.write("class/hwmon/hwmon2/temp1_label", " Tctl \n");

    sysfs.write("class/hwmon/hwmon3/temp1_input", "27000\n");

    sysfs.write("class/thermal/thermal_zone0/type", "x86_pkg_temp\n");
    sysfs.write("class/thermal/thermal_zone0/temp", "44000\n");

    const auto readings = SensorReader{sysfs.root()}.read();
    CHECK(readings.size() == 5);

    const auto* package = find_reading(readings, "hwmon0/temp1_input");
    CHECK(package != nullptr);
    if (package != nullptr) {
        CHECK(package->label == "Package id 0");
        CHECK(package->source == "coretemp");
        CHECK(package->celsius == 45.0);
    }

    const auto* nvme = find_reading(readings, "hwmon1/temp1_input");
    CHECK(nvme != nullptr);
    if (nvme != nullptr) {
        CHECK(nvme->label == "nvme temp1");
        CHECK(nvme->source == "nvme");
        CHECK(nvme->celsius == 38.0);
    }
    CHECK(find_reading(readings, "hwmon1/temp2_input") == nullptr);

    const auto* k10temp = find_reading(readings, "hwmon2/temp1_input");
    CHECK(k10temp != nullptr);
    if (k10temp != nullptr) {
        CHECK(k10temp->label == "Tctl");
        CHECK(k10temp->celsius == -5.5);
    }

    const auto* unnamed = find_reading(readings, "hwmon3/temp1_input");
    CHECK(unnamed != nullptr);
    if (unnamed != nullptr) {
        CHECK(unnamed->label == "hwmon3 temp1");
        CHECK(unnamed->source == "hwmon3");
    }

    const auto* zone = find_reading(readings, "thermal_zone0");
    CHECK(zone != nullptr);
    if (zone != nullptr) {
        CHECK(zone->label == "x86_pkg_temp");
        CHECK(zone->source == "thermal");
        CHECK(zone->celsius == 44.0);
    }
}

void test_missing_sysfs_returns_empty() {
    FakeSysfs sysfs;
    CHECK(SensorReader{sysfs.root()}.read().empty());
}

void test_empty_files_are_skipped() {
    FakeSysfs sysfs;
    sysfs.write("class/hwmon/hwmon0/name", "test\n");
    sysfs.write("class/hwmon/hwmon0/temp1_input", "");
    sysfs.write("class/thermal/thermal_zone0/temp", "\n");

    CHECK(SensorReader{sysfs.root()}.read().empty());
}

} // namespace

void run_sensor_reader_tests() {
    test_hwmon_and_thermal_discovery();
    test_missing_sysfs_returns_empty();
    test_empty_files_are_skipped();
}
