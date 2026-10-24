// SmartShell -- Logging Sink: File Sink with Rotation
#pragma once

#include "../sink.hpp"
#include "../../foundation/types.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <mutex>
#include <filesystem>

namespace ss {

/// Writes log records to a file with automatic size-based rotation.
/// Thread-safe: uses internal mutex.
class FileSink final : public ILogSink {
public:
    struct Options {
        std::filesystem::path path;
        usize maxFileSizeBytes {10ULL * 1024 * 1024}; // 10 MB
        int   maxFiles         {5};
    };

    explicit FileSink(Options opts) : m_opts{std::move(opts)} {
        openFile();
    }

    ~FileSink() override {
        std::lock_guard<std::mutex> lk{m_mutex};
        if (m_file.is_open()) m_file.close();
    }

    void write(const LogRecord& record) noexcept override {
        try {
            std::lock_guard<std::mutex> lk{m_mutex};
            if (!m_file.is_open()) return;

            std::string line = format(record);
            m_file << line;
            m_bytesWritten += line.size();

            if (m_bytesWritten >= m_opts.maxFileSizeBytes) {
                rotate();
            }
        } catch (...) {}
    }

    void flush() noexcept override {
        try {
            std::lock_guard<std::mutex> lk{m_mutex};
            if (m_file.is_open()) m_file.flush();
        } catch (...) {}
    }

private:
    Options    m_opts;
    std::ofstream m_file;
    usize      m_bytesWritten{0};
    std::mutex m_mutex;

    void openFile() {
        m_file.open(m_opts.path, std::ios::app | std::ios::out);
        if (m_file.is_open()) {
            // Measure existing size
            m_file.seekp(0, std::ios::end);
            m_bytesWritten = static_cast<usize>(m_file.tellp());
        }
    }

    void rotate() {
        m_file.close();

        // Shift existing rotated files: .4 -> removed, .3 -> .4, etc.
        for (int i = m_opts.maxFiles - 1; i >= 1; --i) {
            auto old_path = m_opts.path;
            old_path += "." + std::to_string(i);
            auto new_path = m_opts.path;
            new_path += "." + std::to_string(i + 1);
            if (std::filesystem::exists(old_path)) {
                std::error_code ec;
                std::filesystem::rename(old_path, new_path, ec);
            }
        }

        // .log -> .log.1
        auto rotated = m_opts.path;
        rotated += ".1";
        std::error_code ec;
        std::filesystem::rename(m_opts.path, rotated, ec);

        // Open fresh file
        m_bytesWritten = 0;
        openFile();
    }

    static std::string format(const LogRecord& record) {
        std::ostringstream oss;
        auto tt = std::chrono::system_clock::to_time_t(record.timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            record.timestamp.time_since_epoch()) % 1000;
        std::tm tm_buf{};
#ifdef _WIN32
        ::gmtime_s(&tm_buf, &tt);
#else
        ::gmtime_r(&tt, &tm_buf);
#endif
        oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S") << '.'
            << std::setfill('0') << std::setw(3) << ms.count() << 'Z';

        oss << " [" << toString(record.level)   << "]"
            << " [" << toString(record.category) << "]";

        if (record.sensitive) {
            oss << " <sensitive record - content redacted>\n";
        } else {
            oss << " " << record.message << "\n";
        }
        return oss.str();
    }
};

} // namespace ss
