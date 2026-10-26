#pragma once

#include <iostream>
#include <string_view>

namespace tempmonitor::test {

inline int checks = 0;
inline int failures = 0;

inline void check(bool condition, std::string_view expression, std::string_view file, int line) {
    ++checks;
    if (condition) {
        return;
    }
    ++failures;
    std::cerr << file << ':' << line << ": check failed: " << expression << '\n';
}

inline int summarize() {
    if (failures == 0) {
        std::cout << checks << " checks passed\n";
        return 0;
    }
    std::cerr << failures << " of " << checks << " checks failed\n";
    return 1;
}

}

#define CHECK(condition) \
    ::tempmonitor::test::check(static_cast<bool>(condition), #condition, __FILE__, __LINE__)
