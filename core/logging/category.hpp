// SmartShell -- Core Logging: Log Category Enumeration
#pragma once

namespace ss {

/// Subsystem categories for log records. Enables per-category filtering.
enum class LogCategory : int {
    Application   = 0,
    Core          = 1,
    Platform      = 2,
    Process       = 3,
    Filesystem    = 4,
    Configuration = 5,
    Security      = 6,
    Session       = 7,
    Terminal      = 8,
    Shell         = 9,
    Ui            = 10,
    Diagnostics   = 11,
};

[[nodiscard]] const char* toString(LogCategory cat) noexcept;

} // namespace ss
