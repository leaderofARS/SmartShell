// SmartShell -- Platform Windows: Environment Variable Abstraction (Header)
//
// Provides explicit, Unicode-safe access to process environment variables.
// Security note: this layer does NOT log variable values automatically;
// environment variables may contain tokens, credentials, or secrets.
#pragma once

#include "../../../core/text/encoding.hpp"
#include "../../../core/errors/result.hpp"
#include <optional>
#include <map>
#include <vector>
#include <string>

namespace ss::platform::windows {

class Environment {
public:
    // ---- Single-variable access --------------------------------------------

    /// Read an environment variable. Returns nullopt if not set.
    [[nodiscard]] static std::optional<Utf8String> getVariable(Utf8View name);

    /// Set an environment variable in the current process.
    [[nodiscard]] static Status setVariable(Utf8View name, Utf8View value);

    /// Remove an environment variable from the current process.
    [[nodiscard]] static Status removeVariable(Utf8View name);

    // ---- Snapshot ----------------------------------------------------------

    /// Return all current-process environment variables as a UTF-8 map.
    /// Key ordering is case-insensitive on Windows but map is case-sensitive here.
    [[nodiscard]] static Result<std::map<Utf8String, Utf8String>> getAll();

    // ---- Current directory -------------------------------------------------

    [[nodiscard]] static Result<Utf8String> getCurrentDirectory();
    [[nodiscard]] static Status             setCurrentDirectory(Utf8View path);
};

// ---------------------------------------------------------------------------
// EnvironmentBlock
// Builds a Win32-style environment block (null-separated, double-null terminated)
// for passing to CreateProcessW.
// ---------------------------------------------------------------------------
class EnvironmentBlock {
public:
    EnvironmentBlock() = default;

    /// Inherit all current-process variables as the base.
    [[nodiscard]] static Result<EnvironmentBlock> fromCurrentProcess();

    /// Set or override a variable in this block.
    EnvironmentBlock& set(Utf8View name, Utf8View value);

    /// Remove a variable from this block.
    EnvironmentBlock& remove(Utf8View name);

    /// Build the raw wide-char block suitable for CreateProcessW's lpEnvironment.
    /// Returns an owning buffer; the block is double-null terminated.
    [[nodiscard]] std::vector<wchar_t> build() const;

private:
    std::map<Utf8String, Utf8String> m_vars;
};

} // namespace ss::platform::windows
