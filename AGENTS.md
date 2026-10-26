# AGENTS.md

Instructions for AI agents working in this repository.

## Project

`tempmonitor` is a Linux desktop application that displays hardware temperatures in a GTK4 UI. It reads sensors directly from sysfs; it does not require root and does not use the network.

## Stack

- C++20
- GTK4 through gtkmm-4.0
- CMake (>= 3.28) + Ninja
- Linux only
- No shelling out to `sensors`, `lm-sensors`, or other binaries

## Build and run

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/tempmonitor
```

Install dependencies:

```bash
# Debian / Ubuntu
sudo apt install build-essential cmake ninja-build pkg-config libgtkmm-4.0-dev

# Fedora
sudo dnf install gcc-c++ cmake ninja-build pkgconf-pkg-config gtkmm4.0-devel
```

Release build:

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

## Tests and checks

```bash
ctest --test-dir build --output-on-failure
clang-format --dry-run --Werror src/**/*.cpp src/**/*.hpp tests/**/*.cpp
clang-tidy -p build src/**/*.cpp
```

If a check tool is unavailable, say so rather than silently skipping it. A change is done only when the Debug build is warning-free and `ctest` passes.

## Layout

```text
CMakeLists.txt
src/
  main.cpp            Gtk::Application entry point
  app/                application window and refresh scheduling
  sensors/            sysfs discovery and parsing (no GTK dependencies)
  ui/                 gtkmm widgets, views, formatting
tests/                unit tests for sensor parsing and formatting
```

Keep `sensors/` free of GTK includes so it remains unit-testable.

## Architecture rules

- Read temperatures from `/sys/class/hwmon/hwmon*/temp*_input` and `/sys/class/thermal/thermal_zone*/temp`.
- Files contain millidegrees Celsius; divide by 1000.0. Parse defensively and skip malformed or unreadable entries.
- Prefer `temp*_label` for display names, falling back to the chip name and input index.
- Never block the UI thread. Read sensors on a worker (for example `std::jthread`) and deliver results to the UI via `Glib::Dispatcher` or an equivalent gtkmm-safe mechanism.
- Default refresh interval is 1 second. Make it configurable and cancel/join the worker cleanly on shutdown.
- Handle missing sensors gracefully: show a clear "No temperature sensors found" state instead of crashing or logging repeatedly.
- Use `std::optional`, `std::filesystem`, and RAII. Do not let exceptions escape GTK callbacks.
- Use gtkmm types (`Glib::RefPtr`, `sigc::signal`, `Gtk::make_managed`) rather than raw GTK C APIs where gtkmm offers an equivalent.
- Do not add a background daemon, DBus service, tray icon, or network calls unless explicitly requested.

## Style

- Format with `clang-format`; 4-space indentation, 100-column limit.
- Classes and structs: `PascalCase`. Functions and variables: `snake_case`. Members: trailing underscore.
- Wrap code in `namespace tempmonitor`.
- `#pragma once` in headers.
- Prefer `auto` only when the type is obvious from the initializer.
- No comments except for non-obvious hardware quirks; keep public headers self-documenting.
- Keep UI strings in one place so localization can be added later.

## Robustness and security

- Run as an unprivileged user; sysfs reads are read-only.
- Bound all file reads and validate numeric input before converting.
- Never log sensitive host information such as serial numbers, usernames, or full sysfs dumps.
- Avoid unbounded polling; stop timers and threads when the window closes.

## Definition of done

- `cmake --build build` succeeds with `-Wall -Wextra -Wpedantic` enabled.
- `ctest --test-dir build --output-on-failure` passes.
- `clang-format` reports no changes.
- The app starts, shows at least one sensor on a machine with hwmon data, and exits cleanly.
- Missing or partially readable sensor files do not crash the app.
