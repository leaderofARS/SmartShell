// SmartShell — Core Errors: Error Category Enumeration
#pragma once

namespace ss {

// ---------------------------------------------------------------------------
// ErrorCategory — the broad classification of any SmartShell error.
//
// Rule: Do not create new categories without a concrete use case.
//       Prefer using the specific code field within a category.
// ---------------------------------------------------------------------------
enum class ErrorCategory : int {
    General             = 0,    ///< Unclassified errors
    InvalidArgument     = 1,    ///< Bad caller-supplied argument
    InvalidState        = 2,    ///< Operation invalid in current state
    NotFound            = 3,    ///< Requested resource not found
    AccessDenied        = 4,    ///< Permission failure
    AlreadyExists       = 5,    ///< Resource already present
    Timeout             = 6,    ///< Operation timed out
    Cancelled           = 7,    ///< Operation was cancelled
    ResourceExhausted   = 8,    ///< Out of memory, handles, or quota
    IoFailure           = 9,    ///< I/O operation failed
    PlatformFailure     = 10,   ///< Windows API / OS failure
    ParseFailure        = 11,   ///< Failed to parse input
    ConfigurationFailure= 12,   ///< Configuration invalid or missing
    Unsupported         = 13,   ///< Feature not supported on this platform/version
    Internal            = 14,   ///< Bug in SmartShell itself
};

[[nodiscard]] const char* toString(ErrorCategory cat) noexcept;

} // namespace ss
