# SmartShell

Native Windows terminal and shell platform. The implementation follows the
layered architecture in [the baseline specification](specifications/INITIAL_SPECS.md)
and deliberately has no third-party runtime dependencies.

## Build

Prerequisites: CMake 3.25+ and a C++23-capable Windows compiler. The supplied
presets use the MSYS2 UCRT64 `MinGW Makefiles` generator, which is available in
this repository's development environment. The Windows SDK is required for the
native platform APIs as implementation begins.

From an MSYS2 UCRT64 shell or a PowerShell session with its tools on `PATH`:

```powershell
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The executable is written to `out/build/debug/bin/Debug/SmartShell.exe` for a
multi-config generator, or `out/build/debug/bin/Debug`-equivalent build output
for the supplied Ninja preset. Use `SmartShell.exe --version` as a basic smoke
test.

Release and diagnostics configurations are also supplied:

```powershell
cmake --preset release
cmake --build --preset release
cmake --install out/build/release

cmake --preset asan
cmake --build --preset asan
```

`asan` requires a compiler with AddressSanitizer support. `release` requests
LTO when the active compiler supports it.

Visual Studio users can configure an equivalent build without changing project
files:

```powershell
cmake -S . -B out/build/msvc-debug -G "Visual Studio 17 2022" -A x64
cmake --build out/build/msvc-debug --config Debug
ctest --test-dir out/build/msvc-debug --build-config Debug --output-on-failure
```

## CMake structure

The build exposes target boundaries matching the product architecture:

- `SmartShell::core`
- `SmartShell::platform_windows`
- `SmartShell::terminal`
- `SmartShell::shell`
- `SmartShell::session`
- `SmartShell::ui`
- `SmartShell` (application)

At Phase 1 bootstrap these are interface targets; add implementation sources to
the owning module and convert its target to a concrete library when that module
begins. Dependencies flow from the application downwards, so core remains
independent of UI and shell semantics.

Project options are intentionally opt-in: `SMARTSHELL_WARNINGS_AS_ERRORS`,
`SMARTSHELL_ENABLE_CLANG_TIDY`, `SMARTSHELL_ENABLE_ASAN`, and
`SMARTSHELL_ENABLE_LTO`.
