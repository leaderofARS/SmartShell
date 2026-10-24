// SmartShell -- Core Diagnostics: Diagnostic Context
//
// DiagnosticContext captures the state at the time of a significant event.
// It is more structured than a log message -- it records the exact component,
// operation, resource, and error involved.
#pragma once

#include "../foundation/types.hpp"
#include "../text/encoding.hpp"
#include "../time/clock.hpp"
#include <optional>
#include <sstream>

namespace ss {

struct DiagnosticContext {
    Utf8String operation;              ///< e.g. "createFile"
    Utf8String component;              ///< e.g. "FileSink", "Environment"
    Utf8String resourceId;             ///< e.g. a file path or handle value
    Utf8String sessionId;              ///< Optional session identifier
    u32        threadId  {0};
    Timestamp  timestamp {WallClock::now()};
    std::optional<u32> nativeError;    ///< Win32 error code if applicable

    [[nodiscard]] Utf8String toString() const {
        std::ostringstream oss;
        oss << "[" << component << "] " << operation;
        if (!resourceId.empty()) oss << " resource=" << resourceId;
        if (!sessionId.empty())  oss << " session="  << sessionId;
        if (nativeError) oss << " nativeErr=" << *nativeError;
        oss << " at=" << formatTimestamp(timestamp);
        oss << " tid=" << threadId;
        return oss.str();
    }
};

// ---------------------------------------------------------------------------
// HealthState -- simple subsystem health reporting (dev builds)
// ---------------------------------------------------------------------------
enum class HealthState { Unknown, Ok, Degraded, Failed };

struct HealthItem {
    Utf8String  subsystem;
    HealthState state {HealthState::Unknown};
    Utf8String  detail;
};

} // namespace ss
