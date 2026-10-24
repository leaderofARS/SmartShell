// SmartShell -- Platform Windows: Environment Variable Abstraction (Implementation)
#include "environment.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ss::platform::windows {

// ---- Environment::getVariable -----------------------------------------------

std::optional<Utf8String> Environment::getVariable(Utf8View name) {
    auto wideName = toUtf16(name);
    if (wideName.isError()) return std::nullopt;

    DWORD size = ::GetEnvironmentVariableW(wideName.value().c_str(), nullptr, 0);
    if (size == 0) return std::nullopt;  // not set or error

    std::wstring buf(size, L'\0');
    DWORD written = ::GetEnvironmentVariableW(wideName.value().c_str(), buf.data(), size);
    if (written == 0 || written >= size) return std::nullopt;
    buf.resize(written);

    auto utf8 = toUtf8(buf);
    if (utf8.isError()) return std::nullopt;
    return utf8.value();
}

// ---- Environment::setVariable -----------------------------------------------

Status Environment::setVariable(Utf8View name, Utf8View value) {
    auto wideName = toUtf16(name);
    if (wideName.isError()) return std::move(wideName.error());
    auto wideVal = toUtf16(value);
    if (wideVal.isError()) return std::move(wideVal.error());

    if (!::SetEnvironmentVariableW(wideName.value().c_str(), wideVal.value().c_str())) {
        return makeLastError("SetEnvironmentVariableW failed", "setVariable",
                             Utf8String{name});
    }
    return Status::ok();
}

// ---- Environment::removeVariable --------------------------------------------

Status Environment::removeVariable(Utf8View name) {
    auto wideName = toUtf16(name);
    if (wideName.isError()) return std::move(wideName.error());

    // Passing nullptr as value removes the variable
    if (!::SetEnvironmentVariableW(wideName.value().c_str(), nullptr)) {
        DWORD err = ::GetLastError();
        if (err == ERROR_ENVVAR_NOT_FOUND) {
            return Status::ok();  // Already absent — not an error
        }
        return makePlatformError(err, "SetEnvironmentVariableW(remove) failed",
                                 "removeVariable", Utf8String{name});
    }
    return Status::ok();
}

// ---- Environment::getAll ----------------------------------------------------

Result<std::map<Utf8String, Utf8String>> Environment::getAll() {
    LPWCH block = ::GetEnvironmentStringsW();
    if (!block) {
        return makeLastError("GetEnvironmentStringsW failed", "getAll");
    }

    std::map<Utf8String, Utf8String> result;
    const wchar_t* p = block;
    while (*p != L'\0') {
        std::wstring entry{p};
        p += entry.size() + 1;

        // Skip entries starting with '=' (Windows internal variables)
        if (!entry.empty() && entry[0] == L'=') continue;

        auto eq = entry.find(L'=');
        if (eq == std::wstring::npos) continue;

        auto keyW = entry.substr(0, eq);
        auto valW = entry.substr(eq + 1);

        auto keyU = toUtf8(keyW);
        auto valU = toUtf8(valW);
        if (keyU.isOk() && valU.isOk()) {
            result[keyU.value()] = valU.value();
        }
    }
    ::FreeEnvironmentStringsW(block);
    return result;
}

// ---- Environment::getCurrentDirectory ---------------------------------------

Result<Utf8String> Environment::getCurrentDirectory() {
    DWORD size = ::GetCurrentDirectoryW(0, nullptr);
    if (size == 0) return makeLastError("GetCurrentDirectoryW failed", "getCurrentDirectory");

    std::wstring buf(size, L'\0');
    DWORD written = ::GetCurrentDirectoryW(size, buf.data());
    if (written == 0 || written >= size) {
        return makeLastError("GetCurrentDirectoryW failed", "getCurrentDirectory");
    }
    buf.resize(written);
    return toUtf8(buf);
}

Status Environment::setCurrentDirectory(Utf8View path) {
    auto wide = toUtf16(path);
    if (wide.isError()) return std::move(wide.error());
    if (!::SetCurrentDirectoryW(wide.value().c_str())) {
        return makeLastError("SetCurrentDirectoryW failed", "setCurrentDirectory",
                             Utf8String{path});
    }
    return Status::ok();
}

// ---- EnvironmentBlock -------------------------------------------------------

Result<EnvironmentBlock> EnvironmentBlock::fromCurrentProcess() {
    auto all = Environment::getAll();
    if (all.isError()) return std::move(all.error());
    EnvironmentBlock block;
    block.m_vars = std::move(all.value());
    return block;
}

EnvironmentBlock& EnvironmentBlock::set(Utf8View name, Utf8View value) {
    m_vars[Utf8String{name}] = Utf8String{value};
    return *this;
}

EnvironmentBlock& EnvironmentBlock::remove(Utf8View name) {
    m_vars.erase(Utf8String{name});
    return *this;
}

std::vector<wchar_t> EnvironmentBlock::build() const {
    std::vector<wchar_t> block;
    for (auto& [k, v] : m_vars) {
        auto entry = k + "=" + v;
        auto wentry = toUtf16(entry);
        if (wentry.isError()) continue;
        for (wchar_t c : wentry.value()) block.push_back(c);
        block.push_back(L'\0');
    }
    block.push_back(L'\0');  // double-null terminator
    return block;
}

} // namespace ss::platform::windows
