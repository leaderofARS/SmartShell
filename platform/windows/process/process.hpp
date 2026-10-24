// SmartShell -- Platform Windows: Process Abstraction (Header)
//
// ProcessBuilder constructs a process launch request.
// Process is the RAII owner of a running or exited native process.
//
// Security: The executable path and argument list are kept distinct from
// raw command-line strings. The platform layer builds the command line
// internally with proper quoting, preventing injection.
#pragma once

#include "../../../core/errors/result.hpp"
#include "../../../core/identifiers/types.hpp"
#include "../../../core/time/clock.hpp"
#include "../filesystem/path.hpp"
#include "../environment/environment.hpp"
#include "../handles/handle.hpp"
#include <optional>
#include <vector>
#include <string>
#include <chrono>

namespace ss::platform::windows {

// ---------------------------------------------------------------------------
// ProcessState
// ---------------------------------------------------------------------------
enum class ProcessState {
    Created,
    Running,
    Exited,
    Terminated,
    Failed,
};

// ---------------------------------------------------------------------------
// Process (RAII owner of a native process)
// ---------------------------------------------------------------------------
class Process {
public:
    Process() = default;
    ~Process() = default;

    // Non-copyable; movable
    Process(const Process&)            = delete;
    Process& operator=(const Process&) = delete;
    Process(Process&&)                 = default;
    Process& operator=(Process&&)      = default;

    // ---- Identity ----------------------------------------------------------
    [[nodiscard]] ProcessId   processId()  const noexcept { return m_pid; }
    [[nodiscard]] ProcessState state()     const noexcept { return m_state; }
    [[nodiscard]] bool        isRunning()  const noexcept { return m_state == ProcessState::Running; }

    // ---- Lifecycle ---------------------------------------------------------

    /// Block until the process exits or the timeout elapses.
    /// Returns the exit code on success, or Error on timeout/failure.
    [[nodiscard]] Result<int> waitForExit(
        std::optional<Duration> timeout = std::nullopt);

    /// Non-blocking: returns exit code if already exited, nullopt if still running.
    [[nodiscard]] std::optional<int> tryGetExitCode();

    /// Request termination (TerminateProcess). Does not block.
    [[nodiscard]] Status terminate();

    /// Borrow the raw process handle (do not close it).
    [[nodiscard]] HANDLE nativeHandle() const noexcept { return m_processHandle.get(); }

private:
    friend class ProcessBuilder;

    ProcessId   m_pid;
    ProcessState m_state{ProcessState::Created};
    NullHandle   m_processHandle;  // owns the process HANDLE
    NullHandle   m_threadHandle;   // owns the primary thread HANDLE

    explicit Process(ProcessId pid, NullHandle ph, NullHandle th)
        : m_pid{pid}
        , m_state{ProcessState::Running}
        , m_processHandle{std::move(ph)}
        , m_threadHandle{std::move(th)}
    {}
};

// ---------------------------------------------------------------------------
// ProcessBuilder
// ---------------------------------------------------------------------------
class ProcessBuilder {
public:
    ProcessBuilder() = default;

    ProcessBuilder& executable(WindowsPath path) {
        m_executable = std::move(path);
        return *this;
    }

    ProcessBuilder& arguments(std::vector<Utf8String> args) {
        m_arguments = std::move(args);
        return *this;
    }

    ProcessBuilder& addArgument(Utf8String arg) {
        m_arguments.push_back(std::move(arg));
        return *this;
    }

    ProcessBuilder& workingDirectory(WindowsPath dir) {
        m_workingDir = std::move(dir);
        return *this;
    }

    ProcessBuilder& environment(EnvironmentBlock env) {
        m_env = std::move(env);
        return *this;
    }

    ProcessBuilder& inheritHandles(bool inherit) {
        m_inheritHandles = inherit;
        return *this;
    }

    /// Launch the process. Returns a Process object on success.
    [[nodiscard]] Result<Process> build();

private:
    WindowsPath              m_executable;
    std::vector<Utf8String>  m_arguments;
    std::optional<WindowsPath>       m_workingDir;
    std::optional<EnvironmentBlock>  m_env;
    bool                             m_inheritHandles{false};

    // Build a properly-quoted Windows command line from executable + arguments
    [[nodiscard]] static Utf16String buildCommandLine(
        const WindowsPath& exe, const std::vector<Utf8String>& args);
};

} // namespace ss::platform::windows
