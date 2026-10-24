// SmartShell -- Unit Tests: Time
#include "../test_framework.hpp"
#include "../../core/time/clock.hpp"
#include <thread>
#include <chrono>

using namespace ss;
using namespace ss::test;

SS_TEST_CASE("time/wallclock_now_advances") {
    auto t1 = WallClock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto t2 = WallClock::now();
    SS_CHECK(t2 > t1);
}

SS_TEST_CASE("time/monotonic_now_advances") {
    auto t1 = MonotonicClock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto t2 = MonotonicClock::now();
    SS_CHECK(t2 > t1);
}

SS_TEST_CASE("time/monotonic_elapsed") {
    auto start = MonotonicClock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto elapsed = MonotonicClock::elapsed(start);
    // Should be at least 5ms (generous tolerance for slow CI)
    SS_CHECK(elapsed >= std::chrono::milliseconds(5));
}

SS_TEST_CASE("time/formatTimestamp_produces_iso8601") {
    auto ts = WallClock::now();
    auto s  = formatTimestamp(ts);
    // Must have 'T' separator and 'Z' UTC marker
    SS_CHECK(s.find('T') != std::string::npos);
    SS_CHECK(s.back() == 'Z');
    // Must be at least 20 chars: "YYYY-MM-DDTHH:MM:SS."
    SS_CHECK(s.size() >= 20u);
}

SS_TEST_CASE("time/duration_arithmetic") {
    Duration d1 = std::chrono::seconds(1);
    Duration d2 = std::chrono::milliseconds(500);
    SS_CHECK(d1 > d2);
    SS_CHECK(d1 + d2 == std::chrono::milliseconds(1500));
}

int main() { return ss::test::runAll(); }
