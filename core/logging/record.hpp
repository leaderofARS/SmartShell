// SmartShell -- Core Logging: Log Record
#pragma once

#include "level.hpp"
#include "category.hpp"
#include "../foundation/types.hpp"
#include "../text/encoding.hpp"
#include <chrono>

namespace ss {

/// A single immutable log entry produced by Logger and delivered to sinks.
struct LogRecord {
    LogLevel    level     {LogLevel::Info};
    LogCategory category  {LogCategory::Core};
    Utf8String  message;
    std::chrono::system_clock::time_point timestamp;
    u32         threadId  {0};
    u32         processId {0};
    bool        sensitive {false}; ///< If true, sinks must not persist/expose raw message
};

} // namespace ss
