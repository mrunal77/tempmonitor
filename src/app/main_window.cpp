#include "app/main_window.hpp"

#include "ui/formatting.hpp"
#include "ui/strings.hpp"

#include <glibmm/markup.h>
#include <gtkmm/enums.h>
#include <gtkmm/widget.h>

#include <utility>

namespace tempmonitor {

MainWindow::MainWindow(AppConfig config) : poller_(config.refresh_interval) {
    set_title(strings::app_title);
    set_default_size(460, 560);
    set_titlebar(header_);

    title_.set_text(strings::app_title);
    header_.set_title_widget(title_);
    refresh_button_.set_icon_name("view-refresh-symbolic");
    refresh_button_.set_tooltip_text(strings::refresh_tooltip);
    refresh_button_.signal_clicked().connect([this] { poller_.request_refresh(); });
    header_.pack_end(refresh_button_);

    root_.set_spacing(8);
    root_.set_margin_top(12);
    root_.set_margin_bottom(12);
    root_.set_margin_start(12);
    root_.set_margin_end(12);

    scrolled_.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scrolled_.set_vexpand(true);
    scrolled_.set_child(group_container_);
    root_.append(scrolled_);

    status_.set_halign(Gtk::Align::START);
    status_.set_text(strings::scanning);
    root_.append(status_);

    set_child(root_);

    signal_close_request().connect(
        [this] {
            poller_.stop();
            return false;
        },
        false);

    poller_.signal_results().connect([this] { on_results(); });
    poller_.start();
}

MainWindow::~MainWindow() {
    poller_.stop();
}

void MainWindow::on_results() {
    auto result = poller_.take_latest();
    if (!result) {
        return;
    }

    if (!result->error.empty()) {
        set_status(strings::read_error);
        return;
    }

    update_sensors(result->sensors);

    if (result->sensors.empty()) {
        set_status(strings::no_sensors);
    } else {
        set_status(format_sensor_count(result->sensors.size()));
    }
}

void MainWindow::update_sensors(const std::vector<TemperatureReading>& sensors) {
    while (auto* child = group_container_.get_first_child()) {
        group_container_.remove(*child);
    }

    if (sensors.empty()) {
        auto* row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 12);
        row->set_margin_top(8);
        row->set_margin_bottom(8);
        row->set_margin_start(12);
        row->set_margin_end(12);

        auto* label = Gtk::make_managed<Gtk::Label>();
        label->set_text(strings::no_sensors);
        label->set_halign(Gtk::Align::START);
        row->append(*label);
        group_container_.append(*row);
        return;
    }

    std::map<std::string, std::vector<TemperatureReading>> groups;
    for (const auto& sensor : sensors) {
        groups[sensor.source].push_back(sensor);
    }

    for (auto& [type, readings] : groups) {
        auto* group_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 4);
        group_box->set_margin_top(6);
        group_box->set_margin_bottom(6);
        group_box->set_margin_start(12);
        group_box->set_margin_end(12);

        auto* header = Gtk::make_managed<Gtk::Label>();
        header->set_markup("<b>" + Glib::Markup::escape_text(type) + "</b>");
        header->set_halign(Gtk::Align::START);
        group_box->append(*header);

        for (const auto& sensor : readings) {
            auto* row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 12);
            row->set_margin_top(4);
            row->set_margin_bottom(4);

            auto* name = Gtk::make_managed<Gtk::Label>();
            name->set_halign(Gtk::Align::START);
            name->set_hexpand(true);
            name->set_text(sensor.label);

            auto* value = Gtk::make_managed<Gtk::Label>();
            value->set_halign(Gtk::Align::END);
            value->set_valign(Gtk::Align::CENTER);
            value->set_markup(value_markup(sensor.celsius));

            row->append(*name);
            row->append(*value);
            group_box->append(*row);
        }

        group_container_.append(*group_box);
    }
}

void MainWindow::set_status(const std::string& text) {
    status_.set_text(text);
}

Glib::ustring MainWindow::value_markup(double celsius) {
    Glib::ustring markup = R"(<span size="large" weight="bold">)";
    markup += Glib::Markup::escape_text(Glib::ustring{format_temperature(celsius)});
    markup += "</span>";
    return markup;
}

} // namespace tempmonitor
