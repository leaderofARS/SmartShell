// SmartShell — Core Foundation: Product Version
#pragma once

#include "types.hpp"
#include <string>
#include <compare>

namespace ss {

// ---------------------------------------------------------------------------
// Version struct
// ---------------------------------------------------------------------------
struct Version {
    u16 major{0};
    u16 minor{0};
    u16 patch{0};

    constexpr auto operator<=>(const Version&) const noexcept = default;
    constexpr bool operator==(const Version&) const noexcept = default;

    [[nodiscard]] std::string toString() const {
        return std::to_string(major) + '.' +
               std::to_string(minor) + '.' +
               std::to_string(patch);
    }
};

// ---------------------------------------------------------------------------
// Authoritative product version (single source of truth)
// CMake also has this version in project(VERSION ...) for the binary.
// ---------------------------------------------------------------------------
inline constexpr Version kProductVersion{0, 1, 0};

// Schema/protocol versions (placeholders, will be meaningful in later phases)
inline constexpr u16 kConfigSchemaVersion   = 1;
inline constexpr u16 kShellLanguageVersion  = 0;
inline constexpr u16 kTerminalCapVersion    = 0;

} // namespace ss
