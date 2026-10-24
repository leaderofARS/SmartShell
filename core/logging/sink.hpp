// SmartShell -- Core Logging: Sink Interface
// Thread-safety: Implementations must be internally thread-safe.
#pragma once

#include "record.hpp"

namespace ss {

/// Abstract log sink. Each sink receives formatted records from Logger.
/// Sinks must NOT throw -- swallow or ignore errors internally.
class ILogSink {
public:
    virtual ~ILogSink() = default;

    /// Called by Logger for every record that passes the level filter.
    /// Must be thread-safe. Must not throw.
    virtual void write(const LogRecord& record) noexcept = 0;

    /// Flush any buffered output. Must not throw.
    virtual void flush() noexcept = 0;
};

} // namespace ss
