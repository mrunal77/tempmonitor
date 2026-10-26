#include "test_framework.hpp"

void run_formatting_tests();
void run_sensor_reader_tests();

int main() {
    run_formatting_tests();
    run_sensor_reader_tests();
    return tempmonitor::test::summarize();
}
