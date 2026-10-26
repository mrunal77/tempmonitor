#pragma once

#include "app/app_config.hpp"

#include <gtkmm/application.h>

namespace tempmonitor {

class TemperatureApplication : public Gtk::Application {
public:
    static Glib::RefPtr<TemperatureApplication> create(AppConfig config);
    ~TemperatureApplication() override = default;

protected:
    explicit TemperatureApplication(AppConfig config);
    void on_activate() override;

private:
    AppConfig config_;
};

} // namespace tempmonitor
