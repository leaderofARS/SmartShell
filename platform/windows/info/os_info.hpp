// SmartShell -- Platform Windows: OS Information
#pragma once

#include "../../../core/foundation/types.hpp"
#include "../../../core/text/encoding.hpp"
#include <string>

namespace ss::platform::windows {

enum class Architecture { X64, Arm64, Other };

struct OsInfo {
    u32          majorVersion  {0};
    u32          minorVersion  {0};
    u32          buildNumber   {0};
    Architecture architecture  {Architecture::X64};

    [[nodiscard]] Utf8String toString() const;
    [[nodiscard]] bool isWindows10OrLater() const noexcept {
        return majorVersion >= 10;
    }
    [[nodiscard]] bool isWindows11OrLater() const noexcept {
        return majorVersion >= 10 && buildNumber >= 22000;
    }
};

/// Read OS version information.
/// Uses RtlGetVersion to bypass compat shims.
[[nodiscard]] OsInfo    getOsInfo();

/// Determine current process CPU architecture.
[[nodiscard]] Architecture getCurrentArchitecture() noexcept;

/// Return true if the current process has elevated (administrator) privileges.
[[nodiscard]] bool isElevated() noexcept;

} // namespace ss::platform::windows
