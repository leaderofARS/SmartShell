// SmartShell -- Core Configuration: Hierarchical Path
// Represents a dot-separated key like "terminal.font.size".
#pragma once

#include "../text/encoding.hpp"
#include <vector>
#include <string>
#include <sstream>

namespace ss {

class ConfigPath {
public:
    ConfigPath() = default;

    /// Parse a dot-separated path string.
    /// Empty segments are silently skipped.
    static ConfigPath parse(Utf8View dotSeparated) {
        ConfigPath p;
        Utf8String seg;
        for (char c : dotSeparated) {
            if (c == '.') {
                if (!seg.empty()) { p.m_parts.push_back(seg); seg.clear(); }
            } else {
                seg += c;
            }
        }
        if (!seg.empty()) p.m_parts.push_back(seg);
        return p;
    }

    /// Construct from a vector of parts (no splitting needed).
    static ConfigPath fromParts(std::vector<Utf8String> parts) {
        ConfigPath p;
        p.m_parts = std::move(parts);
        return p;
    }

    [[nodiscard]] const std::vector<Utf8String>& parts() const noexcept { return m_parts; }
    [[nodiscard]] bool empty() const noexcept { return m_parts.empty(); }
    [[nodiscard]] usize size() const noexcept { return m_parts.size(); }

    [[nodiscard]] Utf8String toString() const {
        Utf8String result;
        for (usize i = 0; i < m_parts.size(); ++i) {
            if (i > 0) result += '.';
            result += m_parts[i];
        }
        return result;
    }

    /// Return path with the first component removed.
    [[nodiscard]] ConfigPath tail() const {
        if (m_parts.empty()) return {};
        return fromParts({m_parts.begin() + 1, m_parts.end()});
    }

    [[nodiscard]] const Utf8String& head() const {
        SS_ASSERT(!m_parts.empty(), "ConfigPath::head() on empty path");
        return m_parts[0];
    }

    bool operator==(const ConfigPath& o) const noexcept { return m_parts == o.m_parts; }

private:
    std::vector<Utf8String> m_parts;
};

} // namespace ss
