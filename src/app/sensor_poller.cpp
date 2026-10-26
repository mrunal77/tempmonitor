#include "app/sensor_poller.hpp"

#include <exception>
#include <utility>

namespace tempmonitor {

SensorPoller::SensorPoller(std::chrono::milliseconds interval, SensorReader reader)
    : interval_(interval), reader_(std::move(reader)) {}

SensorPoller::~SensorPoller() {
    stop();
}

void SensorPoller::start() {
    if (worker_.joinable()) {
        return;
    }
    worker_ = std::jthread([this](std::stop_token stop_token) { run(stop_token); });
}

void SensorPoller::stop() {
    if (!worker_.joinable()) {
        return;
    }
    worker_.request_stop();
    wake_condition_.notify_all();
    worker_.join();
}

void SensorPoller::request_refresh() {
    {
        const std::lock_guard lock(wake_mutex_);
        refresh_requested_ = true;
    }
    wake_condition_.notify_all();
}

Glib::Dispatcher& SensorPoller::signal_results() {
    return dispatcher_;
}

std::optional<PollResult> SensorPoller::take_latest() {
    const std::lock_guard lock(result_mutex_);
    auto result = std::move(latest_);
    latest_.reset();
    return result;
}

void SensorPoller::run(std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {
        PollResult result;
        try {
            result.sensors = reader_.read();
        } catch (const std::exception& exception) {
            result.error = exception.what();
        } catch (...) {
            result.error = "unknown error";
        }

        if (stop_token.stop_requested()) {
            break;
        }

        {
            const std::lock_guard lock(result_mutex_);
            latest_ = std::move(result);
        }
        dispatcher_.emit();

        std::unique_lock lock(wake_mutex_);
        wake_condition_.wait_for(lock, stop_token, interval_,
                                 [this] { return refresh_requested_; });
        refresh_requested_ = false;
    }
}

} // namespace tempmonitor
