#pragma once

#include "sensors/sensor_reader.hpp"
#include "sensors/temperature_reading.hpp"

#include <glibmm/dispatcher.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace tempmonitor {

struct PollResult {
    std::vector<TemperatureReading> sensors;
    std::string error;
};

class SensorPoller {
public:
    explicit SensorPoller(std::chrono::milliseconds interval, SensorReader reader = SensorReader{});
    ~SensorPoller();

    SensorPoller(const SensorPoller&) = delete;
    SensorPoller& operator=(const SensorPoller&) = delete;

    void start();
    void stop();
    void request_refresh();

    Glib::Dispatcher& signal_results();
    std::optional<PollResult> take_latest();

private:
    void run(std::stop_token stop_token);

    std::chrono::milliseconds interval_;
    SensorReader reader_;
    Glib::Dispatcher dispatcher_;
    mutable std::mutex result_mutex_;
    std::optional<PollResult> latest_;
    std::mutex wake_mutex_;
    std::condition_variable_any wake_condition_;
    bool refresh_requested_ = false;
    std::jthread worker_;
};

} // namespace tempmonitor
