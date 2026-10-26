#pragma once

#include "app/app_config.hpp"
#include "app/sensor_poller.hpp"
#include "sensors/temperature_reading.hpp"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>

#include <glibmm/ustring.h>

#include <map>
#include <string>
#include <vector>

namespace tempmonitor {

class MainWindow : public Gtk::ApplicationWindow {
public:
    explicit MainWindow(AppConfig config);
    ~MainWindow() override;

private:
    void on_results();
    void update_sensors(const std::vector<TemperatureReading>& sensors);
    void set_status(const std::string& text);
    static Glib::ustring value_markup(double celsius);

    Gtk::HeaderBar header_;
    Gtk::Label title_;
    Gtk::Button refresh_button_;
    Gtk::Box root_{Gtk::Orientation::VERTICAL};
    Gtk::ScrolledWindow scrolled_;
    Gtk::Box group_container_{Gtk::Orientation::VERTICAL};
    Gtk::Label status_;
    SensorPoller poller_;
};

} // namespace tempmonitor
