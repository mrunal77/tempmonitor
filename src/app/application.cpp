#include "app/application.hpp"

#include "app/main_window.hpp"

#include <glibmm/refptr.h>
#include <gtkmm/widget.h>

#include <utility>

namespace tempmonitor {

Glib::RefPtr<TemperatureApplication> TemperatureApplication::create(AppConfig config) {
    return Glib::make_refptr_for_instance<TemperatureApplication>(
        new TemperatureApplication(std::move(config)));
}

TemperatureApplication::TemperatureApplication(AppConfig config)
    : Gtk::Application("io.github.tempmonitor"), config_(std::move(config)) {}

void TemperatureApplication::on_activate() {
    auto* window = Gtk::make_managed<MainWindow>(config_);
    add_window(*window);
    window->present();
}

} // namespace tempmonitor
