cmake_minimum_required(VERSION 3.28)
project(tempmonitor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TEMPMONITOR_BUILD_GUI "Build the GTK4 user interface" ON)

include(CTest)

add_library(tempmonitor_warnings INTERFACE)
target_compile_options(tempmonitor_warnings INTERFACE -Wall -Wextra -Wpedantic)

add_library(tempmonitor_core
    src/sensors/sensor_reader.cpp
    src/ui/formatting.cpp
)
target_include_directories(tempmonitor_core PUBLIC src)
target_link_libraries(tempmonitor_core PRIVATE tempmonitor_warnings)
target_compile_features(tempmonitor_core PUBLIC cxx_std_20)

if(TEMPMONITOR_BUILD_GUI)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTKMM4 REQUIRED IMPORTED_TARGET gtkmm-4.0)

    add_executable(tempmonitor
        src/main.cpp
        src/app/application.cpp
        src/app/main_window.cpp
        src/app/sensor_poller.cpp
    )
    target_link_libraries(tempmonitor PRIVATE tempmonitor_core PkgConfig::GTKMM4 tempmonitor_warnings)
    target_compile_features(tempmonitor PRIVATE cxx_std_20)

    install(TARGETS tempmonitor RUNTIME DESTINATION bin)
endif()

if(BUILD_TESTING)
    add_executable(tempmonitor_tests
        tests/test_main.cpp
        tests/test_formatting.cpp
        tests/test_sensor_reader.cpp
    )
    target_link_libraries(tempmonitor_tests PRIVATE tempmonitor_core tempmonitor_warnings)
    target_include_directories(tempmonitor_tests PRIVATE tests)
    target_compile_features(tempmonitor_tests PRIVATE cxx_std_20)

    add_test(NAME tempmonitor_tests COMMAND tempmonitor_tests)
endif()
