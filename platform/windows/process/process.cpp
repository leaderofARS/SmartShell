// SmartShell -- Platform Windows: Process Abstraction (Implementation)
#include "process.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <sstream>

namespace ss::platform::windows {

// ---- Process::waitForExit ---------------------------------------------------

Result<int> Process::waitForExit(std::optional<Duration> timeout) {
    if (!m_processHandle.valid()) {
        return makeError(ErrorCategory::InvalidState,
                         "Process handle is invalid", "waitForExit");
    }

    DWORD ms = timeout.has_value()
        ? static_cast<DWORD>(
              std::chrono::duration_cast<std::chrono::milliseconds>(*timeout).count())
        : INFINITE;

    DWORD waitResult = ::WaitForSingleObject(m_processHandle.get(), ms);
    if (waitResult == WAIT_TIMEOUT) {
        return makeError(ErrorCategory::Timeout,
                         "Process did not exit within timeout", "waitForExit");
    }
    if (waitResult != WAIT_OBJECT_0) {
        return makeLastError("WaitForSingleObject failed", "waitForExit");
    }

    DWORD exitCode = 0;
    if (!::GetExitCodeProcess(m_processHandle.get(), &exitCode)) {
        return makeLastError("GetExitCodeProcess failed", "waitForExit");
    }

    m_state = ProcessState::Exited;
    return static_cast<int>(exitCode);
}

// ---- Process::tryGetExitCode ------------------------------------------------

std::optional<int> Process::tryGetExitCode() {
    if (!m_processHandle.valid()) return std::nullopt;

    DWORD exitCode = 0;
    if (!::GetExitCodeProcess(m_processHandle.get(), &exitCode)) return std::nullopt;

    if (exitCode == STILL_ACTIVE) {
        // Double-check with a zero-timeout wait
        if (::WaitForSingleObject(m_processHandle.get(), 0) != WAIT_OBJECT_0) {
            return std::nullopt;
        }
        // Re-read now that it's confirmed exited
        ::GetExitCodeProcess(m_processHandle.get(), &exitCode);
    }

    m_state = ProcessState::Exited;
    return static_cast<int>(exitCode);
}

// ---- Process::terminate -----------------------------------------------------

Status Process::terminate() {
    if (!m_processHandle.valid()) {
        return makeError(ErrorCategory::InvalidState,
                         "Process handle is invalid", "terminate");
    }
    if (!::TerminateProcess(m_processHandle.get(), 1)) {
        return makeLastError("TerminateProcess failed", "terminate",
                             m_pid.toString());
    }
    m_state = ProcessState::Terminated;
    return Status::ok();
}

// ---- ProcessBuilder::buildCommandLine ---------------------------------------

Utf16String ProcessBuilder::buildCommandLine(
    const WindowsPath& exe, const std::vector<Utf8String>& args)
{
    // Properly quote a single token for Windows CreateProcess
    auto quoteArg = [](const std::wstring& arg) -> std::wstring {
        // If no spaces/quotes, no quoting needed
        if (arg.find_first_of(L" \t\n\v\"") == std::wstring::npos) {
            return arg;
        }
        std::wstring out = L"\"";
        int slashes = 0;
        for (wchar_t c : arg) {
            if (c == L'\\') {
                ++slashes;
            } else if (c == L'"') {
                // Backslashes before a quote must be doubled
                out.append(static_cast<usize>(slashes * 2), L'\\');
                out += L"\\\"";
                slashes = 0;
            } else {
                out.append(static_cast<usize>(slashes), L'\\');
                out += c;
                slashes = 0;
            }
        }
        out.append(static_cast<usize>(slashes * 2), L'\\');
        out += L'"';
        return out;
    };

    std::wstring cmdLine = quoteArg(exe.toWide());
    for (auto& arg : args) {
        auto warg = toUtf16(arg);
        if (warg.isOk()) {
            cmdLine += L' ';
            cmdLine += quoteArg(warg.value());
        }
    }
    return cmdLine;
}

// ---- ProcessBuilder::build --------------------------------------------------

Result<Process> ProcessBuilder::build() {
    if (m_executable.empty()) {
        return makeError(ErrorCategory::InvalidArgument,
                         "No executable specified", "ProcessBuilder::build");
    }

    Utf16String cmdLine = buildCommandLine(m_executable, m_arguments);

    // Working directory
    const wchar_t* workDir = nullptr;
    std::wstring workDirStr;
    if (m_workingDir.has_value()) {
        workDirStr = m_workingDir->toWide();
        workDir = workDirStr.c_str();
    }

    // Environment block
    void* envBlock = nullptr;
    std::vector<wchar_t> envBuf;
    if (m_env.has_value()) {
        envBuf = m_env->build();
        envBlock = envBuf.data();
    }

    STARTUPINFOW si{};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi{};

    BOOL ok = ::CreateProcessW(
        m_executable.toWide().c_str(),       // lpApplicationName
        cmdLine.data(),                        // lpCommandLine (mutable)
        nullptr,                               // lpProcessAttributes
        nullptr,                               // lpThreadAttributes
        m_inheritHandles ? TRUE : FALSE,       // bInheritHandles
        m_env.has_value() ? CREATE_UNICODE_ENVIRONMENT : 0, // dwCreationFlags
        envBlock,                              // lpEnvironment
        workDir,                               // lpCurrentDirectory
        &si,
        &pi
    );

    if (!ok) {
        return makeLastError("CreateProcessW failed", "ProcessBuilder::build",
                             m_executable.toUtf8Safe());
    }

    // Wrap handles in RAII owners
    NullHandle ph{pi.hProcess};
    NullHandle th{pi.hThread};

    ProcessId pid{static_cast<u32>(pi.dwProcessId)};
    return Process{pid, std::move(ph), std::move(th)};
}

} // namespace ss::platform::windows
