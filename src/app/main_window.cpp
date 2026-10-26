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
    list_.set_selection_mode(Gtk::SelectionMode::NONE);
    scrolled_.set_child(list_);
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

    poller_.signal_results().connect(sigc::mem_fun(*this, &MainWindow::on_results));
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
    std::vector<std::string> ids;
    ids.reserve(sensors.size());
    for (const auto& sensor : sensors) {
        ids.push_back(sensor.id);
    }

    if (!shown_ids_ || *shown_ids_ != ids) {
        rebuild_rows(sensors);
        shown_ids_ = std::move(ids);
        return;
    }

    for (const auto& sensor : sensors) {
        const auto iterator = rows_.find(sensor.id);
        if (iterator == rows_.end()) {
            continue;
        }

        auto& widgets = iterator->second;
        widgets.name->set_text(sensor.label);
        widgets.source->set_text(sensor.source);
        widgets.value->set_markup(value_markup(sensor.celsius));
    }
}

void MainWindow::rebuild_rows(const std::vector<TemperatureReading>& sensors) {
    clear_rows();

    if (sensors.empty()) {
        auto* row = Gtk::make_managed<Gtk::ListBoxRow>();
        auto* label = Gtk::make_managed<Gtk::Label>();
        label->set_text(strings::no_sensors);
        label->set_margin_top(24);
        label->set_margin_bottom(24);
        row->set_child(*label);
        list_.append(*row);
        return;
    }

    for (const auto& sensor : sensors) {
        auto* row = Gtk::make_managed<Gtk::ListBoxRow>();
        auto* content = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 12);
        content->set_margin_top(8);
        content->set_margin_bottom(8);
        content->set_margin_start(12);
        content->set_margin_end(12);

        auto* text = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 2);
        text->set_hexpand(true);

        auto* name = Gtk::make_managed<Gtk::Label>();
        name->set_halign(Gtk::Align::START);
        name->set_text(sensor.label);

        auto* source = Gtk::make_managed<Gtk::Label>();
        source->set_halign(Gtk::Align::START);
        source->set_text(sensor.source);
        source->set_opacity(0.65);

        auto* value = Gtk::make_managed<Gtk::Label>();
        value->set_halign(Gtk::Align::END);
        value->set_valign(Gtk::Align::CENTER);
        value->set_markup(value_markup(sensor.celsius));

        text->append(*name);
        text->append(*source);
        content->append(*text);
        content->append(*value);
        row->set_child(*content);
        list_.append(*row);

        rows_.emplace(sensor.id, RowWidgets{.name = name, .source = source, .value = value});
    }
}

void MainWindow::clear_rows() {
    while (auto* child = list_.get_first_child()) {
        list_.remove(*child);
    }
    rows_.clear();
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
