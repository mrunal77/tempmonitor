#include "app/app_config.hpp"
#include "app/application.hpp"

#include <charconv>
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

constexpr int min_interval_seconds = 1;
constexpr int max_interval_seconds = 3600;

struct ParsedCommandLine {
    std::chrono::milliseconds refresh_interval{1000};
    std::vector<std::string> arguments;
};

std::optional<int> parse_seconds(std::string_view text) {
    int value = 0;
    const auto* first = text.data();
    const auto* last = first + text.size();
    const auto [ptr, error] = std::from_chars(first, last, value);
    if (error != std::errc{} || ptr != last) {
        return std::nullopt;
    }
    if (value < min_interval_seconds || value > max_interval_seconds) {
        return std::nullopt;
    }
    return value;
}

std::optional<ParsedCommandLine> parse_command_line(int argc, char** argv, std::string& error) {
    ParsedCommandLine parsed;
    parsed.arguments.emplace_back(argv[0] != nullptr ? argv[0] : "tempmonitor");

    for (int index = 1; index < argc; ++index) {
        const std::string_view argument{argv[index]};

        if (argument == "--interval") {
            if (index + 1 >= argc) {
                error = "--interval requires a value in seconds";
                return std::nullopt;
            }
            const auto seconds = parse_seconds(argv[++index]);
            if (!seconds) {
                error = "interval must be between 1 and 3600 seconds";
                return std::nullopt;
            }
            parsed.refresh_interval = std::chrono::seconds{*seconds};
            continue;
        }

        if (argument.starts_with("--interval=")) {
            const auto seconds = parse_seconds(argument.substr(11));
            if (!seconds) {
                error = "interval must be between 1 and 3600 seconds";
                return std::nullopt;
            }
            parsed.refresh_interval = std::chrono::seconds{*seconds};
            continue;
        }

        parsed.arguments.emplace_back(argument);
    }

    return parsed;
}

} // namespace

int main(int argc, char** argv) {
    std::string error;
    const auto parsed = parse_command_line(argc, argv, error);
    if (!parsed) {
        std::cerr << "tempmonitor: " << error << '\n';
        return 2;
    }

    std::vector<std::string> arguments = parsed->arguments;
    std::vector<char*> application_arguments;
    application_arguments.reserve(arguments.size());
    for (auto& argument : arguments) {
        application_arguments.push_back(argument.data());
    }
    application_arguments.push_back(nullptr);

    int application_argc = static_cast<int>(application_arguments.size()) - 1;
    char** application_argv = application_arguments.data();

    tempmonitor::AppConfig config;
    config.refresh_interval = parsed->refresh_interval;

    auto application = tempmonitor::TemperatureApplication::create(config);
    return application->run(application_argc, application_argv);
}
