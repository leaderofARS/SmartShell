// SmartShell -- Logging Sink: Console (stderr/stdout)
#pragma once

#include "../sink.hpp"
#include "../level.hpp"
#include "../category.hpp"
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <mutex>

namespace ss {

/// Writes log records to stderr (errors/fatals) or stdout (others).
class ConsoleSink final : public ILogSink {
public:
    explicit ConsoleSink(bool colorize = true) : m_colorize{colorize} {}

    void write(const LogRecord& record) noexcept override {
        try {
            std::lock_guard<std::mutex> lk{m_mutex};
            FILE* dest = (record.level >= LogLevel::Warning) ? stderr : stdout;
            std::string msg = format(record);
            std::fputs(msg.c_str(), dest);
        } catch (...) {}
    }

    void flush() noexcept override {
        std::fflush(stdout);
        std::fflush(stderr);
    }

private:
    bool m_colorize;
    std::mutex m_mutex;

    std::string format(const LogRecord& record) {
        std::ostringstream oss;

        // Timestamp
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

        // Level + category
        oss << " [" << toString(record.level)    << "]"
            << " [" << toString(record.category)  << "]"
            << " " << record.message << "\n";

        return oss.str();
    }
};

} // namespace ss
