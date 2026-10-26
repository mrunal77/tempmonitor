# tempmonitor

A small Linux desktop application that displays hardware temperatures in a GTK4 UI. It reads sensors directly from sysfs, so it needs no root, no network, and no `sensors` binary.

## Features

- Reads temperatures from `/sys/class/hwmon/hwmon*/temp*_input` and `/sys/class/thermal/thermal_zone*/temp`.
- Refreshes on a worker thread and updates the UI through a `Glib::Dispatcher`, so the window never freezes.
- Default refresh interval is 1 second; override with `--interval=SECONDS`.
- Shows a clear "No temperature sensors found" state instead of crashing.
- Skips malformed or unreadable sensor files defensively.

## Stack

- C++20
- GTK4 through gtkmm-4.0
- CMake (>= 3.28) + Ninja
- Linux only

## Build and run

```bash
# Install dependencies (Debian / Ubuntu)
sudo apt install build-essential cmake ninja-build pkg-config libgtkmm-4.0-dev

# Fedora
sudo dnf install gcc-c++ cmake ninja-build pkgconf-pkg-config gtkmm4.0-devel

# Build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run
./build/tempmonitor
```

A release build:

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

## Command-line options

```
tempmonitor [--interval=SECONDS]
```

`--interval` accepts an integer between 1 and 3600 seconds.

## Tests and checks

```bash
ctest --test-dir build --output-on-failure
clang-format --dry-run --Werror src/**/*.cpp src/**/*.hpp tests/**/*.cpp
clang-tidy -p build src/**/*.cpp
```

A change is done only when the Debug build is warning-free and `ctest` passes.

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

The `sensors/` directory has no GTK dependencies so it remains unit-testable.

## Robustness

- Runs as an unprivileged user; sysfs reads are read-only.
- Bounds all file reads and validates numeric input before converting.
- Never logs sensitive host information.
- Stops timers and threads when the window closes.