// SmartShell — Core Text: Encoding Types and Conversion Interface
//
// Architecture:
//   Internal representation: UTF-8 (Utf8String / Utf8View)
//   Windows API boundary  : UTF-16 (Utf16String / Utf16View)
//   Raw binary data       : std::vector<ss::byte>
//
// Conversions are explicit, detected for invalidity, and never silently corrupt.
#pragma once

#include "../foundation/types.hpp"
#include "../errors/result.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <span>

namespace ss {

// Raw byte buffer (not assumed to be valid text)
using ByteBuffer = std::vector<byte>;

// Raw byte span (non-owning)
using ByteSpan = std::span<const byte>;

/// Convert UTF-8 to UTF-16. Returns Error on invalid sequences.
[[nodiscard]] Result<Utf16String> toUtf16(Utf8View utf8);

/// Convert UTF-16 to UTF-8. Returns Error on invalid sequences.
[[nodiscard]] Result<Utf8String>  toUtf8(Utf16View utf16);

} // namespace ss
