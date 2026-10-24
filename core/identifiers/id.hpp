// SmartShell — Core: Strong Identifier Template
// Provides type-safe, non-interchangeable identifier types.
// Cheap to copy, comparable, hashable, loggable.
#pragma once

#include "../foundation/types.hpp"
#include <functional>
#include <string>
#include <limits>

namespace ss {

// ---------------------------------------------------------------------------
// StrongId<Tag, T>
//
// Tag   — an empty struct that distinguishes identifier domains
// T     — the underlying primitive (default u32)
//
// Usage:
//   struct ProcessIdTag {};
//   using ProcessId = StrongId<ProcessIdTag>;
// ---------------------------------------------------------------------------
template<typename Tag, typename T = u32>
class StrongId {
public:
    using ValueType = T;

    // The sentinel "invalid" value
    static constexpr T kInvalid = std::numeric_limits<T>::max();

    // Default-construct to invalid
    constexpr StrongId() noexcept : m_value{kInvalid} {}

    // Explicit construction from underlying value
    constexpr explicit StrongId(T value) noexcept : m_value{value} {}

    // Not implicitly convertible to/from T
    [[nodiscard]] constexpr T value() const noexcept { return m_value; }

    [[nodiscard]] constexpr bool isValid() const noexcept { return m_value != kInvalid; }
    [[nodiscard]] explicit constexpr operator bool() const noexcept { return isValid(); }

    // Comparison
    constexpr bool operator==(const StrongId& rhs) const noexcept = default;
    constexpr bool operator!=(const StrongId& rhs) const noexcept = default;
    constexpr bool operator< (const StrongId& rhs) const noexcept { return m_value < rhs.m_value; }
    constexpr bool operator<=(const StrongId& rhs) const noexcept { return m_value <= rhs.m_value; }
    constexpr bool operator> (const StrongId& rhs) const noexcept { return m_value > rhs.m_value; }
    constexpr bool operator>=(const StrongId& rhs) const noexcept { return m_value >= rhs.m_value; }

    [[nodiscard]] std::string toString() const {
        return std::to_string(m_value);
    }

private:
    T m_value;
};

} // namespace ss

// ---------------------------------------------------------------------------
// std::hash specialisation
// ---------------------------------------------------------------------------
namespace std {
template<typename Tag, typename T>
struct hash<ss::StrongId<Tag, T>> {
    std::size_t operator()(const ss::StrongId<Tag, T>& id) const noexcept {
        return std::hash<T>{}(id.value());
    }
};
} // namespace std
