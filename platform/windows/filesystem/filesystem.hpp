// SmartShell -- Platform Windows: Filesystem Operations (Header)
#pragma once

#include "path.hpp"
#include "../../../core/errors/result.hpp"
#include "../../../core/foundation/types.hpp"
#include <vector>
#include <span>
#include <cstdint>
#include <chrono>
#include <optional>

namespace ss::platform::windows::fs {

struct FileMetadata {
    usize fileSize       {0};
    bool  isDirectory    {false};
    bool  isReadOnly     {false};
    std::chrono::system_clock::time_point lastWriteTime;
};

// ---- Existence / type -------------------------------------------------------
[[nodiscard]] bool       exists(const WindowsPath& path) noexcept;
[[nodiscard]] bool       isFile(const WindowsPath& path) noexcept;
[[nodiscard]] bool       isDirectory(const WindowsPath& path) noexcept;

// ---- Metadata ---------------------------------------------------------------
[[nodiscard]] Result<FileMetadata> getMetadata(const WindowsPath& path);

// ---- File I/O ---------------------------------------------------------------
[[nodiscard]] Result<ByteBuffer> readFile(const WindowsPath& path);
[[nodiscard]] Status             writeFile(const WindowsPath& path,
                                           std::span<const byte> data);
[[nodiscard]] Status             appendToFile(const WindowsPath& path,
                                              std::span<const byte> data);

// Convenience text overloads
[[nodiscard]] Result<Utf8String> readTextFile(const WindowsPath& path);
[[nodiscard]] Status             writeTextFile(const WindowsPath& path, Utf8View text);

// ---- File management --------------------------------------------------------
[[nodiscard]] Status             renameFile(const WindowsPath& from, const WindowsPath& to);
[[nodiscard]] Status             deleteFile(const WindowsPath& path);
[[nodiscard]] Status             copyFile(const WindowsPath& from, const WindowsPath& to,
                                          bool overwrite = false);

// ---- Directories ------------------------------------------------------------
[[nodiscard]] Status             createDirectory(const WindowsPath& path);
[[nodiscard]] Status             createDirectories(const WindowsPath& path);
[[nodiscard]] Status             deleteDirectory(const WindowsPath& path,
                                                 bool recursive = false);
[[nodiscard]] Result<std::vector<WindowsPath>> listDirectory(const WindowsPath& path);

// ---- Temporary resources ----------------------------------------------------
/// Creates a unique temporary directory under the system temp folder.
/// The caller is responsible for deleting it when done.
[[nodiscard]] Result<WindowsPath> createTempDirectory();

// ---- Atomic write -----------------------------------------------------------
/// Write data to a temporary file then atomically rename to target.
/// Partial writes are not visible to readers.
[[nodiscard]] Status atomicWriteFile(const WindowsPath& target,
                                     std::span<const byte> data);

} // namespace ss::platform::windows::fs
