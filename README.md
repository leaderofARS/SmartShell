# SmartShell

A native, high-performance Windows terminal and shell platform engineered in modern C++23. The architecture follows a strict layered design with **zero third-party runtime or test dependencies**, ensuring complete deterministic control over memory, threading, and system boundaries.

Detailed architectural analysis and implementation rationale are documented in the [Phase 1 Technical Report](docs/Phase-1_report.md) and [Baseline Specification](specifications/INITIAL_SPECS.md).

---

## Roadmap & Status

| Phase | Description | Status | Key Deliverables |
| :--- | :--- | :---: | :--- |
| **Phase 0** | **Project Setup & Tooling** | **Completed** | CMake presets, C++23 compiler flags, strict warnings, sanitize/LTO configs. |
| **Phase 1** | **Core Platform & Windows Layer** | **Completed** | Core foundations, UTF-8/UTF-16 engine, handle RAII, process/env/fs, custom test runner. |
| **Phase 2** | **Shell Lexer & Lexical Analysis** | *Planned* | Streaming line reader, tokenization engine, quote/escape handling, span tracking. |
| **Phase 3** | **Parser & Abstract Syntax Tree** | *Planned* | Recursive descent parser, command AST, redirection & pipeline nodes. |
| **Phase 4** | **Execution Engine & Pipelines** | *Planned* | Process pipelines, job control, built-in commands, I/O redirection. |
| **Phase 5** | **Terminal VT Engine** | *Planned* | ANSI/VT100 escape sequence parser, screen buffer, cursor addressing. |
| **Phase 6+** | **Rendering, UI & Session** | *Planned* | DirectWrite/Direct2D text rendering, tabbed session management, UI chrome. |

---

## Architectural Highlights

### 1. Zero External Dependencies
To eliminate supply chain vulnerabilities, binary bloat, and runtime licensing complexities, SmartShell incorporates **no external packages or third-party libraries**. Every subsystem—including text transcoding, JSON-like hierarchical configuration, process lifecycle management, and the unit test framework—is implemented from scratch using the C++23 Standard Library and native Win32 APIs.

### 2. Core Subsystems (`SmartShell::core`)
- **Foundations & Strong Identifiers (`core/foundation`, `core/identifiers`):** Type-safe identifier wrappers (`StrongId<Tag, Underlying>`) preventing semantic domain collisions (e.g., process IDs vs. thread IDs).
- **Strict Encoding Boundary (`core/text`):** Strict UTF-8 validation and lossless bidirectional conversion to UTF-16 (`std::wstring`) using native `MultiByteToWideChar` / `WideCharToMultiByte` with `MB_ERR_INVALID_CHARS`.
- **Monadic Error Model (`core/errors`):** Zero-overhead `Result<T>` and `Status` types with `SS_TRY` propagation, separating domain error codes (`CommonError`, `SystemError`, `ConfigError`, `ProcessError`) from C++ exception mechanisms.
- **Structured Multi-Sink Logger (`core/logging`):** Thread-safe logging engine supporting `ConsoleSink`, `DebugSink` (`OutputDebugStringW`), and `FileSink` with custom log record formatting.
- **Hierarchical Configuration (`core/configuration`):** Six-layer precedence evaluation engine (`Defaults` $\rightarrow$ `Machine` $\rightarrow$ `User` $\rightarrow$ `Profile` $\rightarrow$ `Session` $\rightarrow$ `Runtime`) supporting dotted-key access.
- **Time & Diagnostics (`core/time`, `core/diagnostics`):** High-precision monotonic and wall clocks, structured diagnostic context capture, and cooperative cancellation tokens (`CancellationTokenSource`).

### 3. Windows Platform Abstraction (`SmartShell::platform_windows`)
- **Traits-Based Handle RAII (`platform/windows/handles`):** Policy-based `UniqueHandle<Traits>` wrapper guaranteeing deterministic closure for Win32 handles (`KernelHandle` for `CloseHandle`, `NullHandle` for `INVALID_HANDLE_VALUE` APIs).
- **Filesystem Utilities (`platform/windows/filesystem`):** Safe path manipulation (`WindowsPath`), GUID-based temporary directory lifecycle management, and atomic file writes using temp file replacement.
- **Process & Environment (`platform/windows/process`, `platform/windows/environment`):** `ProcessBuilder` with standard MSVC argument quoting and escaping rules, sorted double-null-terminated environment block generation, and clean process spawning.
- **Threading & Synchronization (`platform/windows/threading`):** Slim Win32 thread wrapper (`CreateThread`, `SetThreadDescription`) and zero-allocation `Mutex` implemented over native `SRWLOCK`.
- **Platform Telemetry (`platform/windows/info`):** Reliable OS version detection bypassing Windows 10/11 application manifest deprecations (`RtlGetVersion`) and token-based UAC elevation checking.

---

## Building and Testing

### Prerequisites
- **CMake:** Version 3.25 or newer.
- **Compiler:** Any C++23-compliant toolchain:
  - GCC 15+ (MSYS2 UCRT64 / MinGW-w64)
  - Clang 17+ (LLVM)
  - MSVC v143+ (Visual Studio 2022 17.10+)
- **OS:** Windows 10 / Windows 11 x64

### Build with CMake Presets (MinGW / UCRT64)

```powershell
# Configure and build Debug configuration
cmake --preset debug
cmake --build --preset debug

# Run full test suite (11 test suites)
ctest --preset debug --output-on-failure
```

```powershell
# Configure and build Release configuration (with Link-Time Optimization)
cmake --preset release
cmake --build --preset release

# Run Release test suite
ctest --preset release
```

### Running the Application

The compiled executable is placed in `out/build/<preset>/bin/`:

```powershell
# Check application version
./out/build/debug/bin/SmartShell.exe --version

# Run headless initialization & diagnostic demonstration
./out/build/debug/bin/SmartShell.exe
```

### Visual Studio (MSVC)

```powershell
cmake -S . -B out/build/msvc-debug -G "Visual Studio 17 2022" -A x64
cmake --build out/build/msvc-debug --config Debug
ctest --test-dir out/build/msvc-debug --build-config Debug --output-on-failure
```

---

## Repository Structure

```text
SmartShell/
├── app/                  # Application entry point, CLI bootstrap, and lifecycle management
├── cmake/                # Compiler warnings, analysis flags, and project options
├── core/                 # Core domain library (errors, text, logging, config, time, diagnostics)
│   ├── configuration/   # 6-layer hierarchical configuration store
│   ├── diagnostics/     # Diagnostic context and health tracking
│   ├── errors/          # Result<T>, Status, error categories
│   ├── events/          # Cancellation token and event dispatching
│   ├── foundation/      # Fixed-width types, assertions, compiler definitions
│   ├── identifiers/     # Strongly-typed identifiers
│   ├── logging/         # Multi-sink thread-safe logging infrastructure
│   ├── text/            # Strict UTF-8 / UTF-16 Win32 boundary engine
│   └── time/            # Monotonic and wall clocks, ISO 8601 formatting
├── docs/                 # Architectural reports, specifications, and deep dives
│   └── Phase-1_report.md# Comprehensive 2,700+ line technical report on Phase 0 & 1
├── platform/             # OS abstraction layer
│   └── windows/          # Win32 platform layer (handles, process, threads, fs, env, info)
├── specifications/       # System specifications and architectural design baselines
└── tests/                # In-house unit testing framework and test suites
    ├── test_framework.hpp# Zero-dependency test registration and assertion framework
    └── unit/             # 10 dedicated test suites covering all core and platform components
```

---

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
