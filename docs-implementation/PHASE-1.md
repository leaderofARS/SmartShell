# SmartShell — Phase 1 Implementation Reference

**Phase:** 1 — Core C++ Platform & Infrastructure
**Status:** Completed
**Branch:** `main`
**Compiler:** GCC 15.2.0 (MSYS2 UCRT64 / MinGW-w64)
**Standard:** C++23
**External Libraries:** None (zero third-party dependencies)
**Prerequisites:** CMake 4.2.1+, Windows SDK, MSYS2 UCRT64 on PATH

---

## Overview

This document is the end-to-end implementation reference for Phase 1. It maps every architectural requirement from `roadmap/PHASE-1.md` to the concrete files, classes, and design decisions that fulfil them in the actual codebase.

Phase 1 builds **no visible terminal, no shell parser, no command executor**. Its sole purpose is to deliver an engineered, testable foundation on which every later SmartShell subsystem depends.

---

## Folder Structure (Phase 1 Scope)

```text
SmartShell/
│
├── app/
│   ├── CMakeLists.txt              ← Application CMake target definition
│   └── main.cpp                    ← Application entry point and bootstrap sequence
│
├── cmake/
│   ├── CompilerWarnings.cmake      ← Strict warning flags (-Wall, -Wextra, -Werror, etc.)
│   ├── ProjectOptions.cmake        ← SMARTSHELL_WARNINGS_AS_ERRORS, ENABLE_ASAN, ENABLE_LTO
│   ├── SmartShellConfig.cmake.in   ← CMake package config template
│   └── StaticAnalysis.cmake        ← clang-tidy integration
│
├── core/
│   ├── CMakeLists.txt              ← core library target (SmartShell::core)
│   │
│   ├── foundation/
│   │   ├── types.hpp               ← Fixed-width integer aliases, text type aliases
│   │   ├── compiler.hpp            ← Compiler detection macros (SS_COMPILER_*)
│   │   ├── assert.hpp              ← SS_ASSERT / SS_ASSERT_MSG debug assertion macros
│   │   └── version.hpp             ← Product version constants (MAJOR, MINOR, PATCH, string)
│   │
│   ├── identifiers/
│   │   ├── id.hpp                  ← StrongId<Tag, T> — type-safe opaque identifier template
│   │   └── types.hpp               ← Concrete ID type aliases (ProcessId, SessionId, etc.)
│   │
│   ├── text/
│   │   ├── encoding.hpp            ← toUtf16() / toUtf8() declaration + Utf8String, Utf16String
│   │   └── encoding.cpp            ← MultiByteToWideChar / WideCharToMultiByte implementation
│   │
│   ├── errors/
│   │   ├── error_category.hpp      ← ErrorCategory enum (General, NotFound, AccessDenied, …)
│   │   ├── error.hpp               ← Error struct (category, code, message, nativeCode, operation)
│   │   ├── error.cpp               ← Error::format(), Error::fromSystem() implementation
│   │   └── result.hpp              ← Result<T>, Status (Result<void>), SS_TRY macro
│   │
│   ├── logging/
│   │   ├── level.hpp               ← LogLevel enum: Trace, Debug, Info, Warning, Error, Fatal
│   │   ├── category.hpp            ← LogCategory enum: Application, Core, Platform, Process, …
│   │   ├── record.hpp              ← LogRecord struct (level, category, message, timestamp, tid)
│   │   ├── sink.hpp                ← ILogSink abstract interface
│   │   ├── logger.hpp              ← Logger class declaration (multi-sink, thread-safe)
│   │   ├── logger.cpp              ← Logger::log(), addSink(), setLevel() implementation
│   │   └── sinks/
│   │       ├── console_sink.hpp    ← ConsoleSink: writes to stdout/stderr
│   │       ├── debug_sink.hpp      ← DebugSink: writes via OutputDebugStringW
│   │       └── file_sink.hpp       ← FileSink: writes to rotating log file
│   │
│   ├── time/
│   │   └── clock.hpp               ← WallClock, MonotonicClock, formatTimestamp()
│   │
│   ├── events/
│   │   ├── event.hpp               ← Event<T> generic typed event / simple callback dispatcher
│   │   └── cancellation.hpp        ← CancellationTokenSource, CancellationToken
│   │
│   ├── configuration/
│   │   ├── value.hpp               ← ConfigValue variant (null, bool, i64, f64, string, array, object)
│   │   ├── path.hpp                ← ConfigPath — dotted key parser ("terminal.font.size" → ["terminal","font","size"])
│   │   ├── node.hpp                ← ConfigNode — recursive tree node backed by std::map
│   │   └── store.hpp               ← ConfigStore — 6-layer precedence evaluation engine
│   │
│   └── diagnostics/
│       └── context.hpp             ← DiagnosticContext struct, HealthState enum
│
├── platform/
│   ├── CMakeLists.txt              ← Platform directory router
│   └── windows/
│       ├── CMakeLists.txt          ← platform_windows library target (SmartShell::platform_windows)
│       │
│       ├── handles/
│       │   └── handle.hpp          ← UniqueHandle<Traits>, KernelHandle, NullHandle traits
│       │
│       ├── filesystem/
│       │   ├── path.hpp            ← WindowsPath: normalization, UTF-8↔UTF-16, extended-length paths
│       │   ├── filesystem.hpp      ← atomicWriteFile(), createTempDirectory(), exists(), etc.
│       │   └── filesystem.cpp      ← Win32 filesystem implementation
│       │
│       ├── environment/
│       │   ├── environment.hpp     ← EnvironmentBlock — query, set, remove, enumerate, build
│       │   └── environment.cpp     ← GetEnvironmentVariableW / SetEnvironmentVariableW wrappers
│       │
│       ├── process/
│       │   ├── process.hpp         ← ProcessBuilder, Process RAII class
│       │   └── process.cpp         ← CreateProcessW, argument quoting, lifecycle management
│       │
│       ├── threading/
│       │   ├── thread.hpp          ← Thread RAII wrapper, Mutex (SRWLOCK-backed)
│       │   └── thread.cpp          ← CreateThread, SetThreadDescription, join implementation
│       │
│       └── info/
│           ├── os_info.hpp         ← OsInfo struct, getOsInfo(), isElevated() declarations
│           └── os_info.cpp         ← RtlGetVersion, TOKEN_ELEVATION implementation
│
├── tests/
│   ├── CMakeLists.txt              ← CTest registration for all 11 test executables
│   ├── test_framework.hpp          ← In-house test runner (SS_TEST_CASE, SS_EXPECT, SS_REQUIRE)
│   └── unit/
│       ├── test_types.cpp          ← Foundation types and integer alias tests
│       ├── test_errors.cpp         ← Error construction, formatting, category, SS_TRY tests
│       ├── test_text.cpp           ← UTF-8 / UTF-16 round-trip, invalid sequence detection
│       ├── test_logging.cpp        ← Logger multi-sink, level filtering, record formatting
│       ├── test_time.cpp           ← WallClock, MonotonicClock, duration measurement
│       ├── test_config.cpp         ← ConfigValue, ConfigPath, ConfigStore precedence tests
│       ├── test_handles.cpp        ← UniqueHandle move semantics, KernelHandle, NullHandle
│       ├── test_filesystem.cpp     ← Create/read/write/delete/rename in temp directory
│       ├── test_environment.cpp    ← Get/set/remove/enumerate environment variables
│       └── test_process.cpp        ← ProcessBuilder launch, wait, exit-code, cleanup
│
├── docs/
│   └── Phase-1_report.md           ← Comprehensive 2,785-line Phase 1 technical report
│
├── docs-implementation/
│   └── PHASE-1.md                  ← This file — implementation reference
│
├── CMakeLists.txt                  ← Root CMake: version, options, subdirectories
├── CMakePresets.json               ← debug / release / asan presets (MinGW Makefiles)
├── .gitignore                      ← Excludes /out/, /build/, roadmap/, IDE files
└── README.md                       ← Project overview, build instructions, roadmap table
```

---

## Module-by-Module Implementation Details

---

### M1. Build System & Toolchain (`cmake/`, `CMakeLists.txt`, `CMakePresets.json`)

**Roadmap requirements:** §6, §7, §8, §97, §193 Milestone 1

#### What was built

The build system uses **CMake 4.2.1** with the **MinGW Makefiles** generator targeting the MSYS2 UCRT64 toolchain.

| File | Responsibility |
| :--- | :--- |
| `CMakeLists.txt` | Root; sets C++23 standard, includes cmake/ scripts, adds all subdirectory targets |
| `CMakePresets.json` | Defines `debug`, `release`, `asan` configure + build presets |
| `cmake/CompilerWarnings.cmake` | Applies `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror` |
| `cmake/ProjectOptions.cmake` | Option flags: `SMARTSHELL_WARNINGS_AS_ERRORS`, `SMARTSHELL_ENABLE_ASAN`, `SMARTSHELL_ENABLE_LTO` |
| `cmake/StaticAnalysis.cmake` | Optional clang-tidy integration via `SMARTSHELL_ENABLE_CLANG_TIDY` |

#### CMake targets produced

| CMake Target | Maps To |
| :--- | :--- |
| `SmartShell::core` | `core/` — headers-only + compiled units |
| `SmartShell::platform_windows` | `platform/windows/` — Win32 layer |
| `SmartShell` | `app/main.cpp` — final executable |
| `SmartShell.test_*` | Each unit test in `tests/unit/` |

#### Presets

```bash
# Configure + build debug (assertions on, no optimisation)
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure

# Configure + build release (LTO enabled, assertions off)
cmake --preset release
cmake --build --preset release
ctest --preset release
```

Output is always placed under `out/build/<preset>/` — never mixed with source.

#### Key engineering decision

Compiler warnings are errors (`-Werror`) for all configurations. This prevents warning accumulation over time. `NOMINMAX` and `WIN32_LEAN_AND_MEAN` are defined in every file touching `<windows.h>`, wrapped in `#ifndef` guards to prevent redefinition warnings from MinGW's `<os_defines.h>`.

---

### M2. Foundation Module (`core/foundation/`)

**Roadmap requirements:** §11, §12, §13, §15, §98, §99, §193 Milestone 2

#### `core/foundation/types.hpp`

Provides fixed-width integer aliases used project-wide. Puts all aliases in the `ss::` namespace.

```cpp
// Integer aliases
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8  = std::int8_t;   using i16 = std::int16_t;
using i32 = std::int32_t;  using i64 = std::int64_t;
using usize = std::size_t;
using isize = std::ptrdiff_t;
using byte  = std::uint8_t;

// Text encoding aliases (also consumed by error.hpp and encoding.hpp)
using Utf8String  = std::string;
using Utf8View    = std::string_view;
using Utf16String = std::wstring;
using Utf16View   = std::wstring_view;
```

> [!NOTE]
> `Utf8String` / `Utf16String` live in `types.hpp` rather than `encoding.hpp` to avoid a circular dependency: `result.hpp` → `error.hpp` → `types.hpp`. Moving text aliases here broke the cycle while keeping the vocabulary types available everywhere.

#### `core/foundation/compiler.hpp`

Compiler-detection macros (`SS_COMPILER_GCC`, `SS_COMPILER_MSVC`, `SS_COMPILER_CLANG`) and platform macros. Wraps `NOMINMAX` and `WIN32_LEAN_AND_MEAN` with `#ifndef` guards.

#### `core/foundation/assert.hpp`

```cpp
// In debug builds: prints file/line/condition and calls __debugbreak() / abort()
// In release builds: compiles to nothing (no overhead)
#define SS_ASSERT(condition, message)
#define SS_ASSERT_MSG(condition, fmt, ...)
```

Assertions cover programmer invariants only — not user/OS failures (those go through the error model).

#### `core/foundation/version.hpp`

```cpp
namespace ss {
    inline constexpr u32 VERSION_MAJOR = 0;
    inline constexpr u32 VERSION_MINOR = 1;
    inline constexpr u32 VERSION_PATCH = 0;
    inline constexpr std::string_view VERSION_STRING = "0.1.0";
    inline constexpr std::string_view PRODUCT_NAME   = "SmartShell";
}
```

The `--version` CLI flag reports `SmartShell 0.1.0`.

---

### M3. Identifier System (`core/identifiers/`)

**Roadmap requirements:** §13, §14, §193 Milestone 2

#### `core/identifiers/id.hpp` — `StrongId<Tag, T>`

```cpp
template<typename Tag, typename T = u64>
class StrongId {
public:
    explicit StrongId(T value);
    T value() const noexcept;
    bool operator==(const StrongId&) const = default;
    bool operator<(const StrongId&) const noexcept;
    // ...
};
```

The `Tag` parameter makes different ID types incompatible at compile time. Accidentally passing a `SessionId` where a `ProcessId` is expected is a compile error.

#### `core/identifiers/types.hpp` — Concrete ID types

```cpp
struct ProcessIdTag {};
struct SessionIdTag {};
struct PaneIdTag    {};
struct TabIdTag     {};
struct JobIdTag     {};

using ProcessId = StrongId<ProcessIdTag, u32>;
using SessionId = StrongId<SessionIdTag, u64>;
using PaneId    = StrongId<PaneIdTag,    u32>;
using TabId     = StrongId<TabIdTag,     u32>;
using JobId     = StrongId<JobIdTag,     u64>;
```

All IDs are cheap to copy (wrapping a plain integer), comparable, and loggable. They are stable for the lifetime of the owning object.

---

### M4. Text & Encoding (`core/text/`)

**Roadmap requirements:** §16, §17, §18, §19, §20, §135, §193 Milestone 5

#### Design principle

The project maintains a strict encoding boundary:

- **Internal representation:** UTF-8 (`Utf8String = std::string`)
- **Win32 API boundary:** UTF-16 (`Utf16String = std::wstring`)
- **Conversions:** occur only at explicitly controlled call sites in `encoding.hpp/.cpp`

No code outside `core/text/` and `platform/windows/` performs raw `MultiByteToWideChar` or `WideCharToMultiByte` calls.

#### `core/text/encoding.hpp`

```cpp
namespace ss::text {
    // UTF-8 → UTF-16 (for Win32 API calls)
    // Returns Error on invalid UTF-8 sequence (MB_ERR_INVALID_CHARS)
    Result<Utf16String> toUtf16(Utf8View utf8);

    // UTF-16 → UTF-8 (from Win32 API results)
    // Returns Error on invalid UTF-16 sequence (WC_ERR_INVALID_CHARS)
    Result<Utf8String>  toUtf8(Utf16View utf16);
}
```

#### `core/text/encoding.cpp`

Calls `MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ...)` and `WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ...)`. Both functions return `ERROR_NO_UNICODE_TRANSLATION` on bad input, which is mapped to `ErrorCategory::InvalidArgument`.

> [!IMPORTANT]
> **C++23 compatibility fix:** `std::filesystem::path::u8string()` returns `std::u8string` (char8_t) in C++23. Converting back to `std::string` requires `reinterpret_cast<const char*>(u8.data())`. All `WindowsPath::toUtf8()` methods use this pattern.

#### Ownership rules (§20)

| Type | Semantics |
| :--- | :--- |
| `Utf8String` | Owned, heap-allocated UTF-8 text |
| `Utf8View` | Non-owning view — must not outlive source |
| `Utf16String` | Owned UTF-16 (Win32 boundary only) |
| `Utf16View` | Non-owning UTF-16 view |

---

### M5. Error Model (`core/errors/`)

**Roadmap requirements:** §27–§33, §84, §173, §193 Milestone 3

#### `core/errors/error_category.hpp`

```cpp
enum class ErrorCategory : u32 {
    General,
    InvalidArgument,
    InvalidState,
    NotFound,
    AccessDenied,
    AlreadyExists,
    Timeout,
    Cancelled,
    ResourceExhausted,
    IoFailure,
    PlatformFailure,
    ParseFailure,
    ConfigurationFailure,
    Unsupported,
    Internal,
};
```

Each category maps to a subset of Windows error codes via `Error::fromSystem()`.

#### `core/errors/error.hpp`

```cpp
struct Error {
    ErrorCategory category;
    i32           code;           // Domain-specific sub-code
    std::string   message;        // Human-readable explanation
    DWORD         nativeCode;     // Raw GetLastError() value (0 if N/A)
    std::string   operation;      // Name of the failing operation

    // Construct from GetLastError() — preserves native code
    static Error fromSystem(std::string operation);

    // Format for logs and diagnostics
    std::string format() const;
};
```

The `nativeCode` field always preserves the original Win32 error. It is never discarded during error propagation.

#### `core/errors/result.hpp` — `Result<T>` and `Status`

```cpp
template<typename T>
class Result {
    std::variant<T, Error> m_storage;
public:
    static Result ok(T value);
    static Result err(Error error);

    [[nodiscard]] bool isOk()    const noexcept;
    [[nodiscard]] bool isError() const noexcept;
    [[nodiscard]] const T&    value() const &;
    [[nodiscard]] const Error& error() const &;
    // ...
};

using Status = Result<void>;    // Success/failure without a value
```

#### `SS_TRY` macro

```cpp
// Propagates errors up the call stack cleanly (like Rust's ? operator)
#define SS_TRY(expr)                            \
    ({                                          \
        auto _r = (expr);                       \
        if (!_r.isOk()) return _r.error();      \
        std::move(_r).value();                  \
    })
```

Usage:
```cpp
Status doWork() {
    auto path   = SS_TRY(resolvePath(input));       // propagates Error if fails
    auto handle = SS_TRY(openFile(path));
    return Status::ok();
}
```

---

### M6. Logging System (`core/logging/`)

**Roadmap requirements:** §34–§41, §109, §193 Milestone 4

#### Architecture

```
Logger (owns sinks, mutex, min-level filter)
  │
  ├── ILogSink (pure virtual interface)
  │      ├── ConsoleSink  → std::cout / std::cerr
  │      ├── DebugSink    → OutputDebugStringW (visible in debugger Output window)
  │      └── FileSink     → rotating file on disk
```

Thread safety: `Logger::log()` acquires an `std::mutex` before dispatching to sinks.

#### `core/logging/level.hpp`

```cpp
enum class LogLevel : u8 {
    Trace   = 0,
    Debug   = 1,
    Info    = 2,
    Warning = 3,
    Error   = 4,
    Fatal   = 5,
};
```

#### `core/logging/category.hpp`

```cpp
enum class LogCategory : u32 {
    Application,
    Core,
    Platform,
    Process,
    Filesystem,
    Configuration,
    Security,
    Session,       // Reserved for future phases
    Terminal,      // Reserved
    Shell,         // Reserved
    Ui,            // Reserved
    Diagnostics,
};
```

#### `core/logging/record.hpp`

```cpp
struct LogRecord {
    LogLevel    level;
    LogCategory category;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    u32         threadId;
    std::string sourceFile;   // __FILE__ in debug builds
    int         sourceLine;   // __LINE__ in debug builds
};
```

#### `core/logging/logger.hpp` / `logger.cpp`

```cpp
class Logger {
public:
    void addSink(std::shared_ptr<ILogSink> sink);
    void setMinLevel(LogLevel level);
    void log(LogRecord record);
    void log(LogLevel level, LogCategory category, std::string_view message);
};
```

Logging failure policy (§41): if a sink throws, the exception is caught and the logger falls back to remaining sinks. A logging failure never crashes SmartShell.

#### Sensitive data (§40)

Log calls that may contain secrets use `LogCategory::Security`. The sink abstraction allows a production deployment to route security-category records to a separate, restricted sink rather than the general log file.

#### Sinks

| Sink | File | Win32 API |
| :--- | :--- | :--- |
| `ConsoleSink` | `sinks/console_sink.hpp` | `std::cout` / `std::cerr` |
| `DebugSink` | `sinks/debug_sink.hpp` | `OutputDebugStringW` |
| `FileSink` | `sinks/file_sink.hpp` | `std::ofstream` with rotation |

---

### M7. Time System (`core/time/`)

**Roadmap requirements:** §42–§46, §193 Milestone 2

#### `core/time/clock.hpp`

```cpp
namespace ss::time {

// Wall-clock: for log timestamps, user-visible dates
struct WallClock {
    static std::chrono::system_clock::time_point now();
};

// Monotonic: for timeouts, durations, performance measurement
// Never goes backwards (unlike system_clock on DST changes)
struct MonotonicClock {
    static std::chrono::steady_clock::time_point now();
};

// ISO 8601 formatted timestamp string: "2026-10-24T21:32:39+05:30"
std::string formatTimestamp(std::chrono::system_clock::time_point tp);

} // namespace ss::time
```

Wall-clock and monotonic clocks are **distinct types** — you cannot accidentally subtract a `system_clock` time_point from a `steady_clock` time_point (the compiler rejects it).

---

### M8. Event System & Cancellation (`core/events/`)

**Roadmap requirements:** §47–§51, §79, §80, §140, §147, §148, §193 Milestone 9

#### `core/events/event.hpp` — `Event<T>`

A lightweight synchronous / callback-based event dispatcher. Subscribers register a `std::function<void(const T&)>` callback. Events are dispatched on the calling thread (synchronous delivery).

```cpp
template<typename T>
class Event {
public:
    using Handler = std::function<void(const T&)>;
    SubscriptionId subscribe(Handler handler);
    void           unsubscribe(SubscriptionId id);
    void           dispatch(const T& payload);
};
```

#### `core/events/cancellation.hpp` — Cooperative Cancellation

```cpp
class CancellationTokenSource {
public:
    void             cancel();              // Signal cancellation
    CancellationToken token() const;        // Hand to operations
};

class CancellationToken {
public:
    [[nodiscard]] bool isCancelled() const noexcept;
};
```

Long-running platform operations (process wait, I/O, search) accept a `CancellationToken` parameter. They poll `isCancelled()` at safe points and return `ErrorCategory::Cancelled` if signalled.

This model is intentionally simple for Phase 1. Asynchronous I/O (overlapped, IOCP) is a later-phase concern.

---

### M9. Configuration System (`core/configuration/`)

**Roadmap requirements:** §52–§58, §110, §151–§153, §176, §193 Milestone 8

#### Design

The configuration system is separated into four layers:

```
ConfigValue  — typed value variant
ConfigPath   — hierarchical key parser
ConfigNode   — recursive tree node
ConfigStore  — 6-layer precedence resolver
```

#### `core/configuration/value.hpp` — `ConfigValue`

```cpp
// Variant: null | bool | i64 | f64 | Utf8String | Array | Object
using ConfigValue = std::variant<
    std::monostate,          // null
    bool,
    i64,
    f64,
    Utf8String,
    std::vector<ConfigValue>,
    std::map<Utf8String, ConfigValue>
>;

bool configValueIsNull(const ConfigValue& v);
bool configValueIs<T>(const ConfigValue& v);
Result<T> configValueGet<T>(const ConfigValue& v);
```

#### `core/configuration/path.hpp` — `ConfigPath`

```cpp
class ConfigPath {
public:
    // "terminal.font.size" → ["terminal", "font", "size"]
    static Result<ConfigPath> parse(Utf8View dotted);

    const std::vector<Utf8String>& parts() const;
    Utf8String                     toString() const;
};
```

#### `core/configuration/node.hpp` — `ConfigNode`

Recursive tree node backed by `std::map<Utf8String, ConfigNode>`. Supports:
- `get(ConfigPath)` — walk the tree and return value
- `set(ConfigPath, ConfigValue)` — walk/create nodes and set value
- `contains(ConfigPath)` — existence check
- `remove(ConfigPath)` — delete a key

#### `core/configuration/store.hpp` — `ConfigStore`

Implements the 6-layer precedence stack from the roadmap (§57):

```
Priority 0 (Lowest) : Defaults    — hardcoded application defaults
Priority 1          : Machine     — system-wide administrator settings
Priority 2          : User        — %APPDATA%/SmartShell/config.json
Priority 3          : Profile     — per terminal-profile overrides
Priority 4          : Session     — active session parameters
Priority 5 (Highest): Runtime     — transient CLI flags (e.g. --font-size=18)
```

```cpp
class ConfigStore {
public:
    void   set(ConfigLayer layer, ConfigPath path, ConfigValue value);
    Result<ConfigValue> get(ConfigPath path) const;        // searches layer 5→0
    Result<ConfigValue> getEffective(ConfigPath path) const;
};
```

#### Validation and mutation rules (§56, §151)

- Validation occurs at `set()` call sites using typed getters — wrong types return `ErrorCategory::ConfigurationFailure`.
- The store is **not thread-safe by design** — it is owned by the application and accessed from a single thread during startup. Session-layer overrides are immutable snapshots (§152, §153).

---

### M10. Diagnostics (`core/diagnostics/`)

**Roadmap requirements:** §92–§96, §193 Milestone 3

#### `core/diagnostics/context.hpp`

```cpp
struct DiagnosticContext {
    Utf8String operation;       // e.g. "CreateProcess"
    Utf8String component;       // e.g. "platform::windows::process"
    Utf8String resourceId;      // Handle value or path
    SessionId  sessionId;
    u32        threadId;
    std::chrono::system_clock::time_point timestamp;
    DWORD      nativeError;     // GetLastError() at point of failure

    std::string format() const;
};

enum class HealthState { Ok, Degraded, Failed };
```

Diagnostics are attached to error log entries at the platform boundary. When a Win32 call fails, the platform layer captures `GetLastError()` immediately and constructs a `DiagnosticContext` before the error travels up the call stack.

---

### M11. Windows Handle RAII (`platform/windows/handles/`)

**Roadmap requirements:** §24–§26, §85, §105, §193 Milestone 6

#### `platform/windows/handles/handle.hpp` — `UniqueHandle<Traits>`

```cpp
// Traits define the invalid sentinel value and the destruction function
struct KernelHandleTraits {
    static HANDLE invalid() noexcept { return nullptr; }
    static void   close(HANDLE h) noexcept { ::CloseHandle(h); }
};

struct NullHandleTraits {
    // For CreateFile, which returns INVALID_HANDLE_VALUE on failure
    static HANDLE invalid() noexcept { return INVALID_HANDLE_VALUE; }
    static void   close(HANDLE h) noexcept { ::CloseHandle(h); }
};

template<typename Traits>
class UniqueHandle {
public:
    explicit UniqueHandle(HANDLE h = Traits::invalid());
    ~UniqueHandle();                           // calls Traits::close if valid
    UniqueHandle(UniqueHandle&&) noexcept;     // transfer ownership
    UniqueHandle& operator=(UniqueHandle&&) noexcept;
    UniqueHandle(const UniqueHandle&) = delete;  // no copy

    [[nodiscard]] bool   isValid() const noexcept;
    [[nodiscard]] HANDLE get()     const noexcept;
    HANDLE               release() noexcept;    // relinquish ownership
    void                 reset(HANDLE h = Traits::invalid()) noexcept;
};

using KernelHandle = UniqueHandle<KernelHandleTraits>;   // null-invalid handles
using NullHandle   = UniqueHandle<NullHandleTraits>;      // INVALID_HANDLE_VALUE handles
```

#### Why two traits?

Windows APIs use **two different sentinel values** for invalid handles:
- `nullptr` (0) — `CreateEventW`, `CreateMutexW`, `OpenProcess`, `CreateThread`
- `INVALID_HANDLE_VALUE` (-1) — `CreateFileW`, `CreateNamedPipeW`

Mixing these is a subtle bug. The traits-based design makes this a compile-time distinction.

#### Double-release protection

The destructor only calls `Traits::close()` if the stored handle is not equal to `Traits::invalid()`. After a `reset()` or `release()`, the handle is set to `invalid()`, so a subsequent destruction is a no-op.

---

### M12. Filesystem Abstraction (`platform/windows/filesystem/`)

**Roadmap requirements:** §59–§64, §106, §133, §134, §178, §193 Milestone 7

#### `platform/windows/filesystem/path.hpp` — `WindowsPath`

```cpp
class WindowsPath {
public:
    static WindowsPath fromUtf8(Utf8View utf8);
    static WindowsPath fromNative(const std::filesystem::path& p);

    Utf8String   toUtf8()      const;   // UTF-16→UTF-8 via reinterpret_cast (C++23 safe)
    Utf16String  toUtf16()     const;   // native wstring for Win32
    bool         isAbsolute()  const;
    bool         isRelative()  const;
    WindowsPath  append(Utf8View component) const;
    WindowsPath  parent()      const;
    Utf8String   filename()    const;
    Utf8String   extension()   const;
    bool         isEmpty()     const;
};
```

Handles UNC paths, drive letters, relative paths, and extended-length (`\\?\`) paths transparently.

#### `platform/windows/filesystem/filesystem.hpp` / `filesystem.cpp`

```cpp
namespace ss::platform::windows::fs {

// Atomic write: write to temp, then MoveFileExW MOVEFILE_REPLACE_EXISTING
Result<void> atomicWriteFile(const WindowsPath& path, Utf8View content);

// GUID-based temp directory, automatically cleaned up by RAII guard
Result<WindowsPath> createTempDirectory();

// Thin wrappers over CreateDirectoryW / DeleteFileW / MoveFileExW
Result<void> createDirectory(const WindowsPath& path);
Result<void> deleteFile(const WindowsPath& path);
Result<void> renameFile(const WindowsPath& from, const WindowsPath& to);
Result<bool> exists(const WindowsPath& path);

// Read entire file into Utf8String
Result<Utf8String> readFileUtf8(const WindowsPath& path);

// Write Utf8String to file (creates or overwrites)
Result<void> writeFileUtf8(const WindowsPath& path, Utf8View content);

} // namespace
```

Temp directories use `CoCreateGuid` + `StringFromGUID2` for uniqueness (requires linking `ole32.lib`, done in `platform/windows/CMakeLists.txt`).

> [!TIP]
> `atomicWriteFile` protects configuration saves from partial-write corruption. If the process crashes mid-write, the original file is untouched.

---

### M13. Environment Abstraction (`platform/windows/environment/`)

**Roadmap requirements:** §65–§68, §107, §141, §193 Milestone 7

#### `platform/windows/environment/environment.hpp` / `environment.cpp`

```cpp
class EnvironmentBlock {
public:
    // Query
    Result<Utf8String> get(Utf8View name) const;
    bool               contains(Utf8View name) const;

    // Mutation
    void set(Utf8View name, Utf8View value);
    void remove(Utf8View name);

    // Enumerate all variables as a sorted map
    std::map<Utf8String, Utf8String> enumerate() const;

    // Construct a double-null-terminated WCHAR block for CreateProcessW
    // Variables are sorted (required by Win32 spec for case-insensitive lookup)
    std::vector<WCHAR> buildNativeBlock() const;

    // Snapshot current process environment
    static EnvironmentBlock current();
};
```

Security policy (§67): `EnvironmentBlock::enumerate()` returns all variable names. Callers must redact sensitive variables (`SECRET_TOKEN`, `GITHUB_PAT`, etc.) before logging. The class itself does not automatically log values.

---

### M14. Process Abstraction (`platform/windows/process/`)

**Roadmap requirements:** §69–§75, §108, §141–§144, §162, §177, §193 Milestone 7

#### `platform/windows/process/process.hpp` / `process.cpp`

```cpp
// Builder pattern — prevents half-constructed process config
class ProcessBuilder {
public:
    ProcessBuilder& executable(Utf8View path);
    ProcessBuilder& arguments(std::vector<Utf8String> args);
    ProcessBuilder& workingDirectory(WindowsPath dir);
    ProcessBuilder& environment(EnvironmentBlock env);
    ProcessBuilder& inheritHandles(bool inherit);

    Result<Process> launch();
};

class Process {
public:
    ~Process();                            // Closes handles on destruction

    [[nodiscard]] ProcessId id()     const;
    [[nodiscard]] bool      isRunning() const;
    [[nodiscard]] Result<u32> exitCode() const;

    Result<void> waitForExit();            // Blocking wait
    Result<bool> tryGetExitCode(u32& out); // Non-blocking poll
    Result<void> terminate(u32 exitCode = 1);
};
```

#### Argument quoting (§143)

Standard MSVC `CommandLineToArgvW`-compatible quoting rules are applied in `ProcessBuilder::launch()`:
- Arguments containing spaces or quotes are wrapped in double quotes.
- Internal `"` characters are escaped as `\\"`.
- Trailing backslashes before a quote are doubled.

This prevents argument injection — the caller passes a typed `std::vector<Utf8String>`, never a raw command line string.

#### Lifecycle (§72)

```
Created → (launch()) → Running → (waitForExit() or terminate()) → Exited
```

The `Process` RAII class closes both the process handle and thread handle from `PROCESS_INFORMATION` on destruction.

---

### M15. Threading & Synchronization (`platform/windows/threading/`)

**Roadmap requirements:** §76, §77, §78, §145, §181, §193 Milestone 7

#### `platform/windows/threading/thread.hpp` / `thread.cpp`

```cpp
class Thread {
public:
    using WorkFn = std::function<void()>;

    Thread() = default;
    explicit Thread(Utf8View name, WorkFn fn);  // Names thread via SetThreadDescription
    ~Thread();                                    // Calls join() if joinable

    Thread(Thread&&) noexcept;
    Thread(const Thread&) = delete;

    void join();
    bool joinable() const noexcept;
    u32  id()       const noexcept;
};

// Zero-allocation mutex backed by Win32 SRWLOCK
// SRWLOCK is initialized statically — no CreateMutex / CloseHandle overhead
class Mutex {
public:
    Mutex() noexcept;
    void lock()   noexcept;
    void unlock() noexcept;

    // std::lock_guard / std::unique_lock compatible
    using LockGuard = std::lock_guard<Mutex>;
};
```

#### Thread naming (§145)

`SetThreadDescription` is loaded at runtime via `GetProcAddress` (available from Windows 10 1607+). On older builds, the call is silently skipped. The GCC pragma `-Wcast-function-type` is suppressed around this `GetProcAddress` cast.

#### Deadlock policy (§78)

Lock ordering for Phase 1:
1. `Logger::m_mutex` — innermost, never acquires other locks while held
2. `ConfigStore` mutation — outermost if config + log are both needed
3. `EnvironmentBlock` — independent, no nesting

---

### M16. OS Information (`platform/windows/info/`)

**Roadmap requirements:** §127–§131, §193 Milestone 7

#### `platform/windows/info/os_info.hpp` / `os_info.cpp`

```cpp
struct OsInfo {
    u32         majorVersion;
    u32         minorVersion;
    u32         buildNumber;
    Utf8String  architecture;   // "x64" | "ARM64" | "x86"
    bool        isElevated;
};

Result<OsInfo> getOsInfo();
Result<bool>   isElevated();
```

#### Why not `GetVersionEx`?

`GetVersionEx` returns a compatibility-shim version (always 6.2 on Windows 10/11 without an application manifest). `RtlGetVersion` from `ntdll.dll` bypasses this shim and returns the real build number.

```cpp
// Loaded dynamically to avoid a hard link against ntdll.dll details
using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
auto fn = reinterpret_cast<RtlGetVersionFn>(
    GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));
```

UAC elevation is detected via `OpenProcessToken` + `GetTokenInformation(TOKEN_ELEVATION)`.

---

### M17. Application Entry (`app/`)

**Roadmap requirements:** §87–§91, §170–§172, §195, §193 Milestone 1

#### `app/main.cpp` — Bootstrap Sequence

The entry point runs the following deterministic startup sequence (roadmap §88):

```
1. wmain(argc, argv) entry
2. SetConsoleOutputCP(CP_UTF8) — ensure UTF-8 console output
3. Parse --version flag → print "SmartShell 0.1.0" and exit
4. OsInfo captured → logged to DebugSink
5. Logger initialized → ConsoleSink + DebugSink added
6. Configuration defaults loaded into ConfigStore
7. Temp workspace directory created via createTempDirectory()
8. Environment snapshot taken
9. Controlled child process launched (ping localhost -n 1)
10. Process exit code waited and recorded
11. Logger flushes
12. Temp workspace cleaned up (RAII guard)
13. wmain returns 0
```

Shutdown reverses initialization (§90): resources are released in reverse construction order. The temp directory RAII guard fires before `wmain` returns.

#### No global mutable state (§5.2)

The `Logger` and `ConfigStore` instances are stack-allocated inside `wmain`. They are passed by reference to subsystems that need them. No `extern Logger g_logger;` pattern exists.

---

### M18. Test Infrastructure (`tests/`)

**Roadmap requirements:** §102–§115, §157, §158, §193 Milestone 10

#### `tests/test_framework.hpp` — In-House Test Runner

Since no external test libraries are permitted (§100), a lightweight test runner is implemented in a single header:

```cpp
// Register a named test case
SS_TEST_CASE("category/name") {
    // test body
}

// Assertions
SS_EXPECT(expr)              // non-fatal: records failure, continues
SS_REQUIRE(expr)             // fatal: stops this test immediately
SS_EXPECT_EQ(a, b)
SS_EXPECT_NE(a, b)
SS_REQUIRE_NO_ERROR(result)  // checks Result<T>::isOk()
```

**Macro fix:** The token-pasting `##__LINE__` construct does not expand `__LINE__` in a single macro level. Two-level concatenation macros are required:
```cpp
#define SS_TEST_CONCAT_IMPL(a, b) a##b
#define SS_TEST_CONCAT(a, b)      SS_TEST_CONCAT_IMPL(a, b)
#define SS_TEST_CASE(name) /* uses SS_TEST_CONCAT(_, __LINE__) */
```

#### Test isolation rules (§113)

- Every test that touches the filesystem creates its own `createTempDirectory()` scope and cleans up on exit.
- Environment tests save and restore modified variables — they never permanently alter the user environment.
- Process tests only launch system utilities (`ping`, `cmd /c exit 0`) with known, controlled arguments.
- No test depends on a machine-specific path, administrator status, or network availability.

#### Test results (actual)

```
debug preset:
  11/11 tests passed   100%   2.95 sec total

release preset:
  11/11 tests passed   100%   1.06 sec total
```

| Test Binary | Coverage |
| :--- | :--- |
| `test_types` | Integer aliases, text aliases, byte type |
| `test_errors` | Error construction, `fromSystem`, formatting, `SS_TRY` propagation |
| `test_text` | UTF-8↔UTF-16 round-trip, invalid sequence detection, empty strings |
| `test_logging` | Multi-sink dispatch, level filtering, record format, category |
| `test_time` | WallClock, MonotonicClock, duration measurement, formatTimestamp |
| `test_config` | ConfigValue typing, ConfigPath parsing, ConfigStore 6-layer precedence |
| `test_handles` | Move semantics, `isValid()`, `release()`, `reset()`, double-close safety |
| `test_filesystem` | Create/write/read/rename/delete in temp dir, `atomicWriteFile`, `exists` |
| `test_environment` | Get/set/remove variables, `buildNativeBlock()` sorting, Unicode values |
| `test_process` | `ProcessBuilder` launch, exit code retrieval, working directory, cleanup |
| `SmartShell.version` | `SmartShell.exe --version` → `SmartShell 0.1.0` (exit 0) |

---

## Dependency Graph

```text
                        app/main.cpp
                             │
                   ┌─────────┴──────────┐
                   │                    │
          SmartShell::core    SmartShell::platform_windows
                   │                    │
       ┌───────────┤          ┌──────────┤
       │           │          │          │
 configuration  logging   filesystem  process
       │           │          │          │
   errors        time      handles   environment
       │           │                    │
  foundation    text ───────────────────┘
  (types.hpp)   (encoding.hpp)

tests → core + platform_windows (read-only dependency)
```

**Dependency rules enforced:**
- `core/` never includes any `platform/` header
- `platform/` may include `core/` headers freely
- `app/` includes both but neither depends on `app/`
- No circular dependencies (compiler verified on every build)

---

## Known Fixes Applied During Implementation

| Issue | File | Fix Applied |
|:---|:---|:---|
| `std::filesystem::u8string()` returns `std::u8string` in C++23 | `platform/windows/filesystem/path.hpp` | `reinterpret_cast<const char*>(u8.data())` |
| `NOMINMAX` redefined by MinGW `<os_defines.h>` | All Win32-touching files | `#ifndef NOMINMAX` guards |
| `SS_TEST_CASE` macro generated duplicate names | `tests/test_framework.hpp` | Two-level `SS_TEST_CONCAT` macros |
| `CoCreateGuid` / `StringFromGUID2` link error | `platform/windows/CMakeLists.txt` | Added `ole32` to `target_link_libraries` |
| `NTSTATUS` undeclared in `os_info.cpp` | `platform/windows/info/os_info.cpp` | Used `LONG(WINAPI*)` signature directly |
| `toUtf16` undeclared in `thread.cpp` | `platform/windows/threading/thread.cpp` | Added `#include "../../../core/text/encoding.hpp"` |
| Wrong `../..` depth in `handle.hpp` include | `platform/windows/handles/handle.hpp` | Fixed to `../../../core/foundation/assert.hpp` |
| Circular dep: `encoding.hpp` ↔ `result.hpp` | `core/foundation/types.hpp` | Moved `Utf8String`/`Utf16String` aliases into `types.hpp` |

---

## Architecture Invariants (from roadmap §197)

| # | Invariant | Enforced By |
|:--|:---|:---|
| 1 | Every native resource has a defined owner | `UniqueHandle<Traits>` RAII |
| 2 | Every Win32 failure preserves native error code | `Error::nativeCode`, `Error::fromSystem()` |
| 3 | Every failing operation has defined failure semantics | `Result<T>` / `Status` return types |
| 4 | UTF-8 / UTF-16 conversions happen at controlled boundaries | `core/text/encoding.hpp` only |
| 5 | Core modules do not depend on UI modules | Enforced by CMake target structure |
| 6 | Win32 mechanisms stay below platform boundary | `platform/windows/` only |
| 7 | No global mutable runtime state | All services stack-allocated in `wmain` |
| 8 | Long-running operations have a cancellation story | `CancellationToken` parameter convention |
| 9 | Tests cannot damage user environment | Temp directories + env variable save/restore |
| 10 | Shutdown releases resources deterministically | RAII destructors in reverse construction order |
| 11 | Configuration data validated before use | `ConfigStore::set()` typed validation |
| 12 | Infrastructure failures diagnosable without debugger | `DiagnosticContext`, structured logging |

---

## Phase 1 Exit Criteria — Verification

| Criterion | Status |
|:---|:---:|
| Clean debug build (0 warnings, 0 errors) | ✅ |
| Clean release build with LTO | ✅ |
| 11/11 unit tests pass (debug) | ✅ |
| 11/11 unit tests pass (release) | ✅ |
| `SmartShell.exe --version` exits 0 | ✅ |
| `SmartShell.exe` headless demo exits 0 | ✅ |
| Dependency direction verified (no reverse deps) | ✅ |
| Resource ownership rules documented | ✅ |
| Threading rules documented | ✅ |
| Platform boundary enforced | ✅ |
| No known resource leaks | ✅ |
| No known silent Win32 error loss | ✅ |
| No third-party libraries | ✅ |
| No global mutable state | ✅ |
| Phase-1_report.md written (2,785 lines) | ✅ |

---

## What Phase 2 Gets from Phase 1

When Phase 2 (Shell Lexer) begins, it can assume the full existence of:

| Facility | Used For in Phase 2 |
|:---|:---|
| `Utf8String` / `Utf8View` | Token text representation |
| `Result<T>` / `Status` / `SS_TRY` | Lexer error propagation |
| `ErrorCategory::ParseFailure` | Tokenization error category |
| `Logger` + `LogCategory::Shell` | Lexer diagnostic output |
| `CancellationToken` | Cancellable scan operations |
| `ConfigStore` | Lexer configuration (max line length, encoding policy) |
| `MonotonicClock` | Lexer throughput measurement |
| `test_framework.hpp` + `SS_TEST_CASE` | Tokenizer unit tests |
| `WindowsPath` | Source file paths in error messages |
| `EnvironmentBlock` | Variable expansion source in Phase 3+ |

Phase 2 engineers write zero infrastructure code. They only write the lexer.

---

## Related Documents

| Document | Location |
|:---|:---|
| Roadmap specification | [`roadmap/PHASE-1.md`](../roadmap/PHASE-1.md) |
| Detailed technical report | [`docs/Phase-1_report.md`](../docs/Phase-1_report.md) |
| Initial architecture specification | [`specifications/INITIAL_SPECS.md`](../specifications/INITIAL_SPECS.md) |
| Phase 2 roadmap | [`roadmap/PHASE-2.md`](../roadmap/PHASE-2.md) |
| Project README | [`README.md`](../README.md) |
