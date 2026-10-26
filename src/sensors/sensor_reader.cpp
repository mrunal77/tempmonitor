#include "sensors/sensor_reader.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace tempmonitor {
namespace {

constexpr std::size_t max_file_size = 512;

std::optional<std::string> read_text_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }

    std::array<char, max_file_size> buffer{};
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    const auto count = file.gcount();
    if (count <= 0) {
        return std::nullopt;
    }

    return std::string(buffer.data(), static_cast<std::size_t>(count));
}

std::string trim(std::string_view text) {
    const auto not_space = [](unsigned char value) { return std::isspace(value) == 0; };
    const auto begin = std::find_if(text.begin(), text.end(), not_space);
    const auto end = std::find_if(text.rbegin(), text.rend(), not_space).base();
    if (begin >= end) {
        return {};
    }
    return std::string(begin, end);
}

std::optional<double> parse_celsius(std::string_view text) {
    const std::string cleaned = trim(text);
    if (cleaned.empty() || cleaned.size() > 32) {
        return std::nullopt;
    }

    double millidegrees = 0.0;
    const auto* first = cleaned.data();
    const auto* last = first + cleaned.size();
    const auto [ptr, error] = std::from_chars(first, last, millidegrees);
    if (error != std::errc{} || ptr != last || !std::isfinite(millidegrees)) {
        return std::nullopt;
    }

    return millidegrees / 1000.0;
}

std::optional<std::string> temp_index(std::string_view filename) {
    if (!filename.starts_with("temp") || !filename.ends_with("_input")) {
        return std::nullopt;
    }

    constexpr std::size_t prefix_size = 4;
    constexpr std::size_t suffix_size = 6;
    if (filename.size() <= prefix_size + suffix_size) {
        return std::nullopt;
    }

    const auto digits = filename.substr(prefix_size, filename.size() - prefix_size - suffix_size);
    const bool valid = std::all_of(digits.begin(), digits.end(),
                                   [](unsigned char value) { return std::isdigit(value) != 0; });
    if (!valid) {
        return std::nullopt;
    }

    return std::string(digits);
}

void collect_hwmon(const std::filesystem::path& sysfs_root,
                   std::vector<TemperatureReading>& readings) {
    const auto base = sysfs_root / "class" / "hwmon";
    std::error_code error;
    std::filesystem::directory_iterator iterator(base, error);
    const std::filesystem::directory_iterator end;

    while (!error && iterator != end) {
        const auto directory = iterator->path();
        iterator.increment(error);

        if (!std::filesystem::is_directory(directory, error)) {
            error.clear();
            continue;
        }

        const std::string chip = trim(read_text_file(directory / "name").value_or(""));
        const std::string chip_name = chip.empty() ? directory.filename().string() : chip;

        std::error_code file_error;
        std::filesystem::directory_iterator file_iterator(directory, file_error);
        const std::filesystem::directory_iterator file_end;

        while (!file_error && file_iterator != file_end) {
            const auto entry = file_iterator->path();
            file_iterator.increment(file_error);

            const auto index = temp_index(entry.filename().string());
            if (!index) {
                continue;
            }

            const auto celsius = parse_celsius(read_text_file(entry).value_or(""));
            if (!celsius) {
                continue;
            }

            const std::string label_path = "temp" + *index + "_label";
            const std::string label = trim(read_text_file(directory / label_path).value_or(""));

            readings.push_back(TemperatureReading{
                .id = directory.filename().string() + "/" + entry.filename().string(),
                .label = label.empty() ? chip_name + " temp" + *index : label,
                .source = chip_name,
                .celsius = *celsius,
            });
        }
    }
}

void collect_thermal(const std::filesystem::path& sysfs_root,
                     std::vector<TemperatureReading>& readings) {
    const auto base = sysfs_root / "class" / "thermal";
    std::error_code error;
    std::filesystem::directory_iterator iterator(base, error);
    const std::filesystem::directory_iterator end;

    while (!error && iterator != end) {
        const auto directory = iterator->path();
        iterator.increment(error);

        if (!std::filesystem::is_directory(directory, error)) {
            error.clear();
            continue;
        }

        const auto celsius = parse_celsius(read_text_file(directory / "temp").value_or(""));
        if (!celsius) {
            continue;
        }

        const std::string type = trim(read_text_file(directory / "type").value_or(""));
        const std::string zone_name = type.empty() ? directory.filename().string() : type;

        readings.push_back(TemperatureReading{
            .id = directory.filename().string(),
            .label = zone_name,
            .source = "thermal",
            .celsius = *celsius,
        });
    }
}

} // namespace

SensorReader::SensorReader(std::filesystem::path sysfs_root) : sysfs_root_(std::move(sysfs_root)) {}

std::vector<TemperatureReading> SensorReader::read() const {
    std::vector<TemperatureReading> readings;
    readings.reserve(16);

    collect_hwmon(sysfs_root_, readings);
    collect_thermal(sysfs_root_, readings);

    std::sort(readings.begin(), readings.end(), [](const auto& left, const auto& right) {
        if (left.label != right.label) {
            return left.label < right.label;
        }
        return left.id < right.id;
    });

    return readings;
}

} // namespace tempmonitor
