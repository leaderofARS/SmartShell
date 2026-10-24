// SmartShell -- Core Logging: Logger Implementation
#include "logger.hpp"
#include <chrono>
#include <sstream>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

namespace ss {

// ---------------------------------------------------------------------------
// toString helpers (implemented here to avoid extra TUs)
// ---------------------------------------------------------------------------
const char* toString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        default:                return "?????";
    }
}

const char* toString(LogCategory cat) noexcept {
    switch (cat) {
        case LogCategory::Application:   return "APP  ";
        case LogCategory::Core:          return "CORE ";
        case LogCategory::Platform:      return "PLAT ";
        case LogCategory::Process:       return "PROC ";
        case LogCategory::Filesystem:    return "FS   ";
        case LogCategory::Configuration: return "CFG  ";
        case LogCategory::Security:      return "SEC  ";
        case LogCategory::Session:       return "SESS ";
        case LogCategory::Terminal:      return "TERM ";
        case LogCategory::Shell:         return "SHLL ";
        case LogCategory::Ui:            return "UI   ";
        case LogCategory::Diagnostics:   return "DIAG ";
        default:                         return "?????";
    }
}

// ---------------------------------------------------------------------------
// Logger
// ---------------------------------------------------------------------------
Logger::~Logger() {
    flush();
}

void Logger::addSink(std::unique_ptr<ILogSink> sink) {
    std::lock_guard<std::mutex> lk{m_mutex};
    m_sinks.push_back(std::move(sink));
}

void Logger::setMinLevel(LogLevel level) noexcept {
    std::lock_guard<std::mutex> lk{m_mutex};
    m_minLevel = level;
}

LogLevel Logger::minLevel() const noexcept {
    std::lock_guard<std::mutex> lk{m_mutex};
    return m_minLevel;
}

void Logger::log(LogLevel level, LogCategory category,
                 Utf8View message, bool sensitive) noexcept {
    try {
        if (static_cast<int>(level) < static_cast<int>(minLevel())) return;

        LogRecord rec;
        rec.level     = level;
        rec.category  = category;
        rec.message   = sensitive ? Utf8String{"<redacted>"} : Utf8String{message};
        rec.timestamp = std::chrono::system_clock::now();
        rec.sensitive = sensitive;

#ifdef _WIN32
        rec.threadId  = static_cast<u32>(::GetCurrentThreadId());
        rec.processId = static_cast<u32>(::GetCurrentProcessId());
#endif

        deliverToSinks(rec);
    } catch (...) {
        // Logging must never propagate exceptions
    }
}

void Logger::flush() noexcept {
    std::lock_guard<std::mutex> lk{m_mutex};
    for (auto& sink : m_sinks) {
        try { sink->flush(); } catch (...) {}
    }
}

void Logger::deliverToSinks(const LogRecord& record) noexcept {
    std::lock_guard<std::mutex> lk{m_mutex};
    for (auto& sink : m_sinks) {
        try { sink->write(record); } catch (...) {}
    }
}

} // namespace ss
