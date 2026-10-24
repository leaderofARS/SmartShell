// SmartShell — Core Errors: Error Implementation
#include "error.hpp"
#include <sstream>
#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace ss {

// ---------------------------------------------------------------------------
// toString(ErrorCategory)
// ---------------------------------------------------------------------------
const char* toString(ErrorCategory cat) noexcept {
    switch (cat) {
        case ErrorCategory::General:              return "General";
        case ErrorCategory::InvalidArgument:      return "InvalidArgument";
        case ErrorCategory::InvalidState:         return "InvalidState";
        case ErrorCategory::NotFound:             return "NotFound";
        case ErrorCategory::AccessDenied:         return "AccessDenied";
        case ErrorCategory::AlreadyExists:        return "AlreadyExists";
        case ErrorCategory::Timeout:              return "Timeout";
        case ErrorCategory::Cancelled:            return "Cancelled";
        case ErrorCategory::ResourceExhausted:    return "ResourceExhausted";
        case ErrorCategory::IoFailure:            return "IoFailure";
        case ErrorCategory::PlatformFailure:      return "PlatformFailure";
        case ErrorCategory::ParseFailure:         return "ParseFailure";
        case ErrorCategory::ConfigurationFailure: return "ConfigurationFailure";
        case ErrorCategory::Unsupported:          return "Unsupported";
        case ErrorCategory::Internal:             return "Internal";
        default:                                  return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// Error::toString()
// ---------------------------------------------------------------------------
Utf8String Error::toString() const {
    std::ostringstream oss;
    oss << '[' << ss::toString(category) << "] " << message;
    if (!operation.empty()) oss << " (op=" << operation << ')';
    if (!context.empty())   oss << " {" << context << '}';
    if (nativeCode)         oss << " [native=" << *nativeCode << ']';
    return oss.str();
}

// ---------------------------------------------------------------------------
// Factory: makeError
// ---------------------------------------------------------------------------
Error makeError(ErrorCategory cat, Utf8View message,
                Utf8View operation, Utf8View context) noexcept {
    return Error{
        .category  = cat,
        .code      = 0,
        .message   = Utf8String{message},
        .nativeCode= std::nullopt,
        .operation = Utf8String{operation},
        .context   = Utf8String{context},
    };
}

// ---------------------------------------------------------------------------
// Factory: makePlatformError (caller provides native code)
// ---------------------------------------------------------------------------
Error makePlatformError(u32 nativeCode, Utf8View message,
                        Utf8View operation, Utf8View context) noexcept {
    // Build a supplementary message from the native code
    std::string msg{message};

#ifdef _WIN32
    // Append Windows error description
    LPSTR buf = nullptr;
    DWORD len = ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, static_cast<DWORD>(nativeCode),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buf), 0, nullptr);
    if (len > 0 && buf) {
        // Strip trailing \r\n
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) --len;
        if (!msg.empty()) msg += ": ";
        msg.append(buf, len);
        ::LocalFree(buf);
    }
#endif

    return Error{
        .category  = ErrorCategory::PlatformFailure,
        .code      = static_cast<int>(nativeCode),
        .message   = std::move(msg),
        .nativeCode= nativeCode,
        .operation = Utf8String{operation},
        .context   = Utf8String{context},
    };
}

// ---------------------------------------------------------------------------
// Factory: makeLastError (reads GetLastError internally, Windows only)
// ---------------------------------------------------------------------------
Error makeLastError(Utf8View message, Utf8View operation, Utf8View context) noexcept {
#ifdef _WIN32
    return makePlatformError(static_cast<u32>(::GetLastError()),
                             message, operation, context);
#else
    return makeError(ErrorCategory::PlatformFailure, message, operation, context);
#endif
}

} // namespace ss
