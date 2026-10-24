// SmartShell — Core Text: Encoding Conversion
// UTF-8 <-> UTF-16 using Win32 MultiByteToWideChar / WideCharToMultiByte.
// All failures are represented as explicit Errors - no silent corruption.
#include "encoding.hpp"
#include "../errors/result.hpp"
#include "../errors/error.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ss {

Result<Utf16String> toUtf16(Utf8View utf8) {
    if (utf8.empty()) return Utf16String{};

    int needed = ::MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        utf8.data(),
        static_cast<int>(utf8.size()),
        nullptr,
        0
    );
    if (needed <= 0) {
        return makeLastError("UTF-8 to UTF-16 conversion failed", "toUtf16");
    }

    Utf16String result(static_cast<usize>(needed), L'\0');
    int written = ::MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        utf8.data(),
        static_cast<int>(utf8.size()),
        result.data(),
        needed
    );
    if (written <= 0) {
        return makeLastError("UTF-8 to UTF-16 conversion failed (write)", "toUtf16");
    }

    result.resize(static_cast<usize>(written));
    return result;
}

Result<Utf8String> toUtf8(Utf16View utf16) {
    if (utf16.empty()) return Utf8String{};

    int needed = ::WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        utf16.data(),
        static_cast<int>(utf16.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );
    if (needed <= 0) {
        return makeLastError("UTF-16 to UTF-8 conversion failed", "toUtf8");
    }

    Utf8String result(static_cast<usize>(needed), '\0');
    int written = ::WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        utf16.data(),
        static_cast<int>(utf16.size()),
        result.data(),
        needed,
        nullptr,
        nullptr
    );
    if (written <= 0) {
        return makeLastError("UTF-16 to UTF-8 conversion failed (write)", "toUtf8");
    }

    result.resize(static_cast<usize>(written));
    return result;
}

} // namespace ss
