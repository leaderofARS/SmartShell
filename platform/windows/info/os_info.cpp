// SmartShell -- Platform Windows: OS Information (Implementation)
#include "os_info.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <sstream>

namespace ss::platform::windows {

// ---- OsInfo::toString -------------------------------------------------------

Utf8String OsInfo::toString() const {
    std::ostringstream oss;
    oss << "Windows " << majorVersion << '.' << minorVersion
        << " Build " << buildNumber;
    switch (architecture) {
        case Architecture::X64:   oss << " x64";   break;
        case Architecture::Arm64: oss << " ARM64"; break;
        default:                  oss << " ?arch";  break;
    }
    if (isWindows11OrLater()) oss << " (Windows 11+)";
    else if (isWindows10OrLater()) oss << " (Windows 10)";
    return oss.str();
}

// ---- getOsInfo --------------------------------------------------------------

OsInfo getOsInfo() {
    OsInfo info;

    // Use RtlGetVersion (ntdll) to get the real version, bypassing app-compat shims.
    using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    static auto rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
        ::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    if (rtlGetVersion) {
        RTL_OSVERSIONINFOW osi{};
        osi.dwOSVersionInfoSize = sizeof(osi);
        if (rtlGetVersion(&osi) == 0 /*STATUS_SUCCESS*/) {
            info.majorVersion = osi.dwMajorVersion;
            info.minorVersion = osi.dwMinorVersion;
            info.buildNumber  = osi.dwBuildNumber;
        }
    } else {
        // Fallback: GetVersionExW (deprecated, may be shimmed)
        OSVERSIONINFOW ovi{};
        ovi.dwOSVersionInfoSize = sizeof(ovi);
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(suppress: 4996)
#endif
        ::GetVersionExW(&ovi);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
        info.majorVersion = ovi.dwMajorVersion;
        info.minorVersion = ovi.dwMinorVersion;
        info.buildNumber  = ovi.dwBuildNumber;
    }

    info.architecture = getCurrentArchitecture();
    return info;
}

// ---- getCurrentArchitecture -------------------------------------------------

Architecture getCurrentArchitecture() noexcept {
    SYSTEM_INFO si{};
    ::GetNativeSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: return Architecture::X64;
        case PROCESSOR_ARCHITECTURE_ARM64: return Architecture::Arm64;
        default:                           return Architecture::Other;
    }
}

// ---- isElevated -------------------------------------------------------------

bool isElevated() noexcept {
    HANDLE token = nullptr;
    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return false;
    }

    TOKEN_ELEVATION elevation{};
    DWORD size = sizeof(elevation);
    BOOL result = ::GetTokenInformation(token, TokenElevation,
                                        &elevation, size, &size);
    ::CloseHandle(token);

    return result && elevation.TokenIsElevated != 0;
}

} // namespace ss::platform::windows
