// SmartShell — Core Errors: Error Type
//
// An Error captures everything needed for diagnosis:
//   - semantic category and specific code
//   - human-readable message
//   - native OS error code (when applicable)
//   - operation and context strings
//
// Errors are value types — cheap to move, explicit to copy.
#pragma once

#include "error_category.hpp"
#include "../foundation/types.hpp"
#include <optional>
#include <string>

namespace ss {

// ---------------------------------------------------------------------------
// Error
// ---------------------------------------------------------------------------
struct Error {
    ErrorCategory   category    {ErrorCategory::General};
    int             code        {0};        ///< Subsystem-specific error code
    Utf8String      message;                ///< Human-readable description
    std::optional<u32> nativeCode;          ///< Win32 / POSIX error code if any
    Utf8String      operation;              ///< What operation failed ("readFile")
    Utf8String      context;                ///< Additional context ("path=/foo/bar")

    [[nodiscard]] Utf8String toString() const;
};

// ---------------------------------------------------------------------------
// Factory helpers
// ---------------------------------------------------------------------------

/// General error with a message
[[nodiscard]] Error makeError(
    ErrorCategory cat,
    Utf8View message,
    Utf8View operation = {},
    Utf8View context = {}) noexcept;

/// Platform error — captures GetLastError() automatically
/// @param nativeCode  Pass GetLastError() result
[[nodiscard]] Error makePlatformError(
    u32 nativeCode,
    Utf8View message,
    Utf8View operation = {},
    Utf8View context = {}) noexcept;

/// Platform error — calls GetLastError() internally (Windows only, included via platform layer)
[[nodiscard]] Error makeLastError(
    Utf8View message,
    Utf8View operation = {},
    Utf8View context = {}) noexcept;

} // namespace ss
