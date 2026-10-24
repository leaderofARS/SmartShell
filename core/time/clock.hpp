// SmartShell -- Core Time: Clock Abstractions and Formatting
//
// WallClock   -- absolute time (system_clock) for logs, timestamps
// MonotonicClock -- monotonic time (steady_clock) for timeouts, measurements
// Duration    -- std::chrono::nanoseconds alias for precise intervals
// Timestamp   -- system_clock::time_point alias
#pragma once

#include "../text/encoding.hpp"
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace ss {

// ---------------------------------------------------------------------------
// Type aliases
// ---------------------------------------------------------------------------
using Duration  = std::chrono::nanoseconds;
using Timestamp = std::chrono::system_clock::time_point;
using MonotonicPoint = std::chrono::steady_clock::time_point;

// ---------------------------------------------------------------------------
// WallClock -- reads absolute (wall-clock) time
// ---------------------------------------------------------------------------
struct WallClock {
    [[nodiscard]] static Timestamp now() noexcept {
        return std::chrono::system_clock::now();
    }
};

// ---------------------------------------------------------------------------
// MonotonicClock -- reads monotonic time; never goes backwards
// Use for: timeouts, durations, performance measurements.
// ---------------------------------------------------------------------------
struct MonotonicClock {
    [[nodiscard]] static MonotonicPoint now() noexcept {
        return std::chrono::steady_clock::now();
    }

    /// Elapsed duration since a previously recorded point.
    [[nodiscard]] static Duration elapsed(MonotonicPoint since) noexcept {
        return std::chrono::duration_cast<Duration>(MonotonicClock::now() - since);
    }
};

// ---------------------------------------------------------------------------
// Timestamp formatting -- produces ISO 8601 UTC string
// e.g. "2024-10-21T13:30:00.123Z"
// ---------------------------------------------------------------------------
[[nodiscard]] inline Utf8String formatTimestamp(Timestamp ts) {
    auto tt = std::chrono::system_clock::to_time_t(ts);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        ts.time_since_epoch()) % 1000;
    std::tm tm_buf{};
#ifdef _WIN32
    ::gmtime_s(&tm_buf, &tt);
#else
    ::gmtime_r(&tt, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S") << '.'
        << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return oss.str();
}

} // namespace ss
