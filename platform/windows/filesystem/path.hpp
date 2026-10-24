// SmartShell -- Platform Windows: Filesystem Path
#pragma once

#include "../../../core/text/encoding.hpp"
#include "../../../core/errors/result.hpp"
#include <filesystem>
#include <string>

namespace ss::platform::windows {

/// WindowsPath wraps std::filesystem::path with SmartShell conventions:
///   - Public API accepts/returns UTF-8 (Utf8String/Utf8View)
///   - Internally uses std::filesystem::path (wchar_t on Windows)
///   - Explicit conversions only; no implicit encoding magic
class WindowsPath {
public:
    WindowsPath() = default;

    /// Construct from a wide-character path (native Windows).
    explicit WindowsPath(const std::filesystem::path& p) : m_path{p} {}
    explicit WindowsPath(std::filesystem::path&& p) : m_path{std::move(p)} {}

    /// Parse a UTF-8 string into a WindowsPath.
    /// Returns Error if the UTF-8 is invalid or the path cannot be constructed.
    [[nodiscard]] static Result<WindowsPath> fromUtf8(Utf8View utf8) {
        auto wide = toUtf16(utf8);
        if (wide.isError()) return std::move(wide.error());
        return WindowsPath{std::filesystem::path{wide.value()}};
    }

    // ---- Properties --------------------------------------------------------

    [[nodiscard]] bool isAbsolute()  const { return m_path.is_absolute(); }
    [[nodiscard]] bool isRelative()  const { return m_path.is_relative(); }
    [[nodiscard]] bool empty()       const { return m_path.empty(); }

    [[nodiscard]] WindowsPath parent()    const { return WindowsPath{m_path.parent_path()}; }
    [[nodiscard]] WindowsPath filename()  const { return WindowsPath{m_path.filename()}; }
    [[nodiscard]] WindowsPath extension() const { return WindowsPath{m_path.extension()}; }
    [[nodiscard]] WindowsPath stem()      const { return WindowsPath{m_path.stem()}; }

    [[nodiscard]] WindowsPath append(const WindowsPath& child) const {
        return WindowsPath{m_path / child.m_path};
    }

    WindowsPath operator/(const WindowsPath& rhs) const { return append(rhs); }

    /// Lexically normalize (resolve . and .. without I/O).
    [[nodiscard]] WindowsPath normalized() const {
        return WindowsPath{m_path.lexically_normal()};
    }

    /// Canonical form — requires path to exist; resolves symlinks.
    /// Returns Error if path doesn't exist.
    [[nodiscard]] Result<WindowsPath> canonical() const {
        std::error_code ec;
        auto canon = std::filesystem::canonical(m_path, ec);
        if (ec) {
            return makeError(ErrorCategory::IoFailure,
                             "canonical() failed: " + ec.message(),
                             "canonical", toUtf8Safe());
        }
        return WindowsPath{canon};
    }

    // ---- String conversion --------------------------------------------------

    /// UTF-8 representation.
    [[nodiscard]] Utf8String toUtf8() const {
        auto u8 = m_path.u8string();
        return Utf8String(reinterpret_cast<const char*>(u8.data()), u8.size());
    }

    /// Safe UTF-8 conversion (no throw).
    [[nodiscard]] Utf8String toUtf8Safe() const noexcept {
        try {
            auto u8 = m_path.u8string();
            return Utf8String(reinterpret_cast<const char*>(u8.data()), u8.size());
        } catch (...) {
            return "<path-encoding-error>";
        }
    }

    /// Native wide-string (for Win32 APIs).
    [[nodiscard]] const std::wstring& toWide() const { return m_path.native(); }

    /// Underlying std::filesystem::path (for interop with stdlib).
    [[nodiscard]] const std::filesystem::path& native() const noexcept { return m_path; }

    // ---- Comparison --------------------------------------------------------
    bool operator==(const WindowsPath& o) const noexcept { return m_path == o.m_path; }
    bool operator!=(const WindowsPath& o) const noexcept { return m_path != o.m_path; }
    bool operator< (const WindowsPath& o) const noexcept { return m_path <  o.m_path; }

private:
    std::filesystem::path m_path;
};

} // namespace ss::platform::windows
