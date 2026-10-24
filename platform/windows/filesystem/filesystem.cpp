// SmartShell -- Platform Windows: Filesystem Operations (Implementation)
#include "filesystem.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objbase.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

namespace ss::platform::windows::fs {

namespace {

// Map std::error_code to ss::Error
Error fromStdError(const std::error_code& ec, Utf8View op, Utf8View ctx = {}) {
    auto msg = ec.message();
    ErrorCategory cat = ErrorCategory::IoFailure;
    if (ec == std::errc::no_such_file_or_directory)  cat = ErrorCategory::NotFound;
    else if (ec == std::errc::permission_denied)      cat = ErrorCategory::AccessDenied;
    else if (ec == std::errc::file_exists)            cat = ErrorCategory::AlreadyExists;
    return Error{
        .category = cat,
        .code = ec.value(),
        .message = msg,
        .nativeCode = std::nullopt,
        .operation = Utf8String{op},
        .context = Utf8String{ctx}
    };
}

} // anonymous namespace

// ---- Existence / type -------------------------------------------------------

bool exists(const WindowsPath& path) noexcept {
    std::error_code ec;
    return std::filesystem::exists(path.native(), ec);
}

bool isFile(const WindowsPath& path) noexcept {
    std::error_code ec;
    return std::filesystem::is_regular_file(path.native(), ec);
}

bool isDirectory(const WindowsPath& path) noexcept {
    std::error_code ec;
    return std::filesystem::is_directory(path.native(), ec);
}

// ---- Metadata ---------------------------------------------------------------

Result<FileMetadata> getMetadata(const WindowsPath& path) {
    std::error_code ec;
    auto status = std::filesystem::status(path.native(), ec);
    if (ec) return fromStdError(ec, "getMetadata", path.toUtf8Safe());

    FileMetadata m;
    m.isDirectory = std::filesystem::is_directory(status);
    m.isReadOnly  = (status.permissions() & std::filesystem::perms::owner_write)
                    == std::filesystem::perms::none;

    if (!m.isDirectory) {
        m.fileSize = static_cast<usize>(std::filesystem::file_size(path.native(), ec));
        if (ec) return fromStdError(ec, "getMetadata/fileSize", path.toUtf8Safe());
    }

    auto lwt = std::filesystem::last_write_time(path.native(), ec);
    if (!ec) {
        // Convert file_time_type to system_clock::time_point
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            lwt - std::filesystem::file_time_type::clock::now()
            + std::chrono::system_clock::now());
        m.lastWriteTime = sctp;
    }

    return m;
}

// ---- File I/O ---------------------------------------------------------------

Result<ByteBuffer> readFile(const WindowsPath& path) {
    std::ifstream file(path.native(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return makeLastError("Failed to open file for reading", "readFile",
                             path.toUtf8Safe());
    }
    auto size = file.tellg();
    if (size < 0) {
        return makeLastError("Failed to seek file", "readFile", path.toUtf8Safe());
    }
    file.seekg(0, std::ios::beg);

    ByteBuffer buf(static_cast<usize>(size));
    if (size > 0) {
        if (!file.read(reinterpret_cast<char*>(buf.data()), size)) {
            return makeLastError("Failed to read file", "readFile", path.toUtf8Safe());
        }
    }
    return buf;
}

Status writeFile(const WindowsPath& path, std::span<const byte> data) {
    std::ofstream file(path.native(), std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return makeLastError("Failed to open file for writing", "writeFile",
                             path.toUtf8Safe());
    }
    if (!data.empty()) {
        file.write(reinterpret_cast<const char*>(data.data()),
                   static_cast<std::streamsize>(data.size()));
        if (!file) {
            return makeLastError("Failed to write file", "writeFile", path.toUtf8Safe());
        }
    }
    return Status::ok();
}

Status appendToFile(const WindowsPath& path, std::span<const byte> data) {
    std::ofstream file(path.native(), std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        return makeLastError("Failed to open file for append", "appendToFile",
                             path.toUtf8Safe());
    }
    if (!data.empty()) {
        file.write(reinterpret_cast<const char*>(data.data()),
                   static_cast<std::streamsize>(data.size()));
        if (!file) {
            return makeLastError("Failed to append to file", "appendToFile",
                                 path.toUtf8Safe());
        }
    }
    return Status::ok();
}

Result<Utf8String> readTextFile(const WindowsPath& path) {
    auto buf = readFile(path);
    if (buf.isError()) return std::move(buf.error());
    auto& data = buf.value();
    return Utf8String{reinterpret_cast<const char*>(data.data()), data.size()};
}

Status writeTextFile(const WindowsPath& path, Utf8View text) {
    auto dataSpan = std::span<const byte>(
        reinterpret_cast<const byte*>(text.data()), text.size());
    return writeFile(path, dataSpan);
}

// ---- File management --------------------------------------------------------

Status renameFile(const WindowsPath& from, const WindowsPath& to) {
    std::error_code ec;
    std::filesystem::rename(from.native(), to.native(), ec);
    if (ec) return fromStdError(ec, "renameFile",
                                from.toUtf8Safe() + " -> " + to.toUtf8Safe());
    return Status::ok();
}

Status deleteFile(const WindowsPath& path) {
    std::error_code ec;
    std::filesystem::remove(path.native(), ec);
    if (ec) return fromStdError(ec, "deleteFile", path.toUtf8Safe());
    return Status::ok();
}

Status copyFile(const WindowsPath& from, const WindowsPath& to, bool overwrite) {
    std::error_code ec;
    auto opts = overwrite
        ? std::filesystem::copy_options::overwrite_existing
        : std::filesystem::copy_options::none;
    std::filesystem::copy_file(from.native(), to.native(), opts, ec);
    if (ec) return fromStdError(ec, "copyFile",
                                from.toUtf8Safe() + " -> " + to.toUtf8Safe());
    return Status::ok();
}

// ---- Directories ------------------------------------------------------------

Status createDirectory(const WindowsPath& path) {
    std::error_code ec;
    std::filesystem::create_directory(path.native(), ec);
    if (ec) return fromStdError(ec, "createDirectory", path.toUtf8Safe());
    return Status::ok();
}

Status createDirectories(const WindowsPath& path) {
    std::error_code ec;
    std::filesystem::create_directories(path.native(), ec);
    if (ec) return fromStdError(ec, "createDirectories", path.toUtf8Safe());
    return Status::ok();
}

Status deleteDirectory(const WindowsPath& path, bool recursive) {
    std::error_code ec;
    if (recursive) {
        std::filesystem::remove_all(path.native(), ec);
    } else {
        std::filesystem::remove(path.native(), ec);
    }
    if (ec) return fromStdError(ec, "deleteDirectory", path.toUtf8Safe());
    return Status::ok();
}

Result<std::vector<WindowsPath>> listDirectory(const WindowsPath& path) {
    std::error_code ec;
    std::vector<WindowsPath> entries;
    for (auto& entry : std::filesystem::directory_iterator(path.native(), ec)) {
        entries.emplace_back(entry.path());
    }
    if (ec) return fromStdError(ec, "listDirectory", path.toUtf8Safe());
    return entries;
}

// ---- Temporary resources ----------------------------------------------------

Result<WindowsPath> createTempDirectory() {
    wchar_t tempBase[MAX_PATH + 1];
    DWORD len = ::GetTempPathW(MAX_PATH + 1, tempBase);
    if (len == 0) {
        return makeLastError("GetTempPathW failed", "createTempDirectory");
    }

    GUID guid{};
    ::CoCreateGuid(&guid);

    // Format: <tempBase>\ss_<hex-guid>
    std::wstring dirPath = tempBase;
    wchar_t guidStr[40];
    ::StringFromGUID2(guid, guidStr, 40);
    // Remove braces and dashes from {XXXXXXXX-...}
    std::wstring guidClean;
    for (auto c : std::wstring(guidStr)) {
        if (c != L'{' && c != L'}' && c != L'-') guidClean += c;
    }
    dirPath += L"ss_" + guidClean;

    std::error_code ec;
    std::filesystem::create_directories(dirPath, ec);
    if (ec) {
        return Error{
            .category = ErrorCategory::IoFailure,
            .code = ec.value(),
            .message = ec.message(),
            .nativeCode = std::nullopt,
            .operation = "createTempDirectory",
            .context = {}
        };
    }
    return WindowsPath{std::filesystem::path{dirPath}};
}

// ---- Atomic write -----------------------------------------------------------

Status atomicWriteFile(const WindowsPath& target, std::span<const byte> data) {
    // Write to target.tmp, then rename over target
    auto tmpPath = WindowsPath{target.native().parent_path() /
                               (target.native().filename().wstring() + L".tmp")};

    auto writeResult = writeFile(tmpPath, data);
    if (writeResult.isError()) return writeResult;

    // Atomic rename (on Windows this replaces the target atomically)
    return renameFile(tmpPath, target);
}

} // namespace ss::platform::windows::fs
