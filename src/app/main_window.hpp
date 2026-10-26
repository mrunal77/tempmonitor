#pragma once

#include "app/app_config.hpp"
#include "app/sensor_poller.hpp"
#include "sensors/temperature_reading.hpp"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/scrolledwindow.h>

#include <glibmm/ustring.h>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace tempmonitor {

class MainWindow : public Gtk::ApplicationWindow {
public:
    explicit MainWindow(AppConfig config);
    ~MainWindow() override;

private:
    struct RowWidgets {
        Gtk::Label* name = nullptr;
        Gtk::Label* source = nullptr;
        Gtk::Label* value = nullptr;
    };

    void on_results();
    void update_sensors(const std::vector<TemperatureReading>& sensors);
    void rebuild_rows(const std::vector<TemperatureReading>& sensors);
    void clear_rows();
    void set_status(const std::string& text);
    static Glib::ustring value_markup(double celsius);

    Gtk::HeaderBar header_;
    Gtk::Label title_;
    Gtk::Button refresh_button_;
    Gtk::Box root_{Gtk::Orientation::VERTICAL};
    Gtk::ScrolledWindow scrolled_;
    Gtk::ListBox list_;
    Gtk::Label status_;
    SensorPoller poller_;
    std::map<std::string, RowWidgets> rows_;
    std::optional<std::vector<std::string>> shown_ids_;
};

} // namespace tempmonitor
