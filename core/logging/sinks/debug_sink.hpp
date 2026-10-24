// SmartShell -- Logging Sink: OutputDebugString (Windows debugger sink)
#pragma once

#include "../sink.hpp"
#include "../../foundation/types.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

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

/// Delivers log records to the Windows debugger via OutputDebugStringA.
/// Active only when a debugger is attached; otherwise a no-op.
class DebugSink final : public ILogSink {
public:
    void write(const LogRecord& record) noexcept override {
#ifdef _WIN32
        if (!::IsDebuggerPresent()) return;
        try {
            auto msg = format(record);
            ::OutputDebugStringA(msg.c_str());
        } catch (...) {}
#endif
    }

    void flush() noexcept override {}

private:
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
        oss << '[';
        oss << std::put_time(&tm_buf, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
        oss << "] [" << toString(record.level)    << "]";
        oss << " [" << toString(record.category)  << "]";
        oss << ' '  << record.message << "\n";
        return oss.str();
    }
};

} // namespace ss
