// SmartShell -- Core Logging: Log Level Enumeration
#pragma once

namespace ss {

/// Severity levels for log records. Higher value = more severe.
enum class LogLevel : int {
    Trace   = 0,  ///< Extremely fine-grained, usually disabled in production
    Debug   = 1,  ///< Development-time diagnostics
    Info    = 2,  ///< Normal operational messages
    Warning = 3,  ///< Unexpected but recoverable condition
    Error   = 4,  ///< Failure that affects an operation
    Fatal   = 5,  ///< Unrecoverable failure — application will terminate
};

[[nodiscard]] const char* toString(LogLevel level) noexcept;

} // namespace ss
