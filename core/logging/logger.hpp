// SmartShell -- Core Logging: Logger
//
// Logger is the central log dispatching service.
// Thread-safety: All public methods are thread-safe.
// Failure policy: If a sink throws (which it must not), the exception is swallowed.
//
// Ownership: Logger is owned by the Application object. Components receive a
// non-owning reference. Logger must outlive all components.
#pragma once

#include "sink.hpp"
#include "record.hpp"
#include "level.hpp"
#include "category.hpp"
#include "../text/encoding.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

namespace ss {

class Logger {
public:
    Logger() = default;
    ~Logger();

    // Non-copyable, non-movable (owned by application, referenced by pointer)
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger&&)      = delete;

    // ---- Sink management ---------------------------------------------------

    /// Add a sink. Logger takes ownership.
    void addSink(std::unique_ptr<ILogSink> sink);

    // ---- Filtering ---------------------------------------------------------

    /// Set the minimum level. Records below this level are dropped before reaching sinks.
    void setMinLevel(LogLevel level) noexcept;

    [[nodiscard]] LogLevel minLevel() const noexcept;

    // ---- Logging -----------------------------------------------------------

    /// Core logging method. Builds a LogRecord and delivers it to all sinks.
    /// @param sensitive If true, sinks must not expose or persist the raw message.
    void log(LogLevel level, LogCategory category,
             Utf8View message, bool sensitive = false) noexcept;

    // Convenience wrappers
    void trace  (LogCategory cat, Utf8View msg) noexcept { log(LogLevel::Trace,   cat, msg); }
    void debug  (LogCategory cat, Utf8View msg) noexcept { log(LogLevel::Debug,   cat, msg); }
    void info   (LogCategory cat, Utf8View msg) noexcept { log(LogLevel::Info,    cat, msg); }
    void warning(LogCategory cat, Utf8View msg) noexcept { log(LogLevel::Warning, cat, msg); }
    void error  (LogCategory cat, Utf8View msg) noexcept { log(LogLevel::Error,   cat, msg); }
    void fatal  (LogCategory cat, Utf8View msg) noexcept { log(LogLevel::Fatal,   cat, msg); }

    /// Flush all sinks.
    void flush() noexcept;

private:
    mutable std::mutex               m_mutex;
    std::vector<std::unique_ptr<ILogSink>> m_sinks;
    LogLevel                         m_minLevel{LogLevel::Info};

    void deliverToSinks(const LogRecord& record) noexcept;
};

// ---------------------------------------------------------------------------
// Logging macros
// ---------------------------------------------------------------------------
// Usage: SS_LOG(logger, LogLevel::Info, LogCategory::Core, "message")
#define SS_LOG(logger_, level_, cat_, msg_) \
    do { \
        if (static_cast<int>(level_) >= static_cast<int>((logger_).minLevel())) { \
            (logger_).log(level_, cat_, msg_); \
        } \
    } while(false)

#define SS_LOG_TRACE(l, c, m)   SS_LOG(l, ::ss::LogLevel::Trace,   c, m)
#define SS_LOG_DEBUG(l, c, m)   SS_LOG(l, ::ss::LogLevel::Debug,   c, m)
#define SS_LOG_INFO(l, c, m)    SS_LOG(l, ::ss::LogLevel::Info,    c, m)
#define SS_LOG_WARN(l, c, m)    SS_LOG(l, ::ss::LogLevel::Warning, c, m)
#define SS_LOG_ERROR(l, c, m)   SS_LOG(l, ::ss::LogLevel::Error,   c, m)
#define SS_LOG_FATAL(l, c, m)   SS_LOG(l, ::ss::LogLevel::Fatal,   c, m)

} // namespace ss
