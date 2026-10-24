// SmartShell -- Unit Tests: Logging
#include "../test_framework.hpp"
#include "../../core/logging/logger.hpp"
#include "../../core/logging/sink.hpp"
#include "../../core/logging/record.hpp"
#include <vector>
#include <string>

using namespace ss;
using namespace ss::test;

// ---- Capture sink for testing -----------------------------------------------
struct CaptureSink final : ILogSink {
    std::vector<LogRecord> records;

    void write(const LogRecord& r) noexcept override {
        records.push_back(r);
    }
    void flush() noexcept override {}
};

// ---- Tests ------------------------------------------------------------------

SS_TEST_CASE("logging/records_delivered_to_sink") {
    Logger logger;
    auto* cap = new CaptureSink;
    logger.addSink(std::unique_ptr<ILogSink>(cap));
    logger.setMinLevel(LogLevel::Trace);

    logger.info(LogCategory::Core, "hello");
    SS_REQUIRE(!cap->records.empty());
    SS_CHECK_EQ(cap->records[0].message, std::string{"hello"});
    SS_CHECK(cap->records[0].level == LogLevel::Info);
    SS_CHECK(cap->records[0].category == LogCategory::Core);
}

SS_TEST_CASE("logging/level_filtering") {
    Logger logger;
    auto* cap = new CaptureSink;
    logger.addSink(std::unique_ptr<ILogSink>(cap));
    logger.setMinLevel(LogLevel::Warning);

    logger.debug(LogCategory::Core, "filtered out");
    logger.info(LogCategory::Core, "also filtered");
    logger.warning(LogCategory::Core, "passes through");

    SS_CHECK_EQ(cap->records.size(), 1u);
    SS_CHECK(cap->records[0].level == LogLevel::Warning);
}

SS_TEST_CASE("logging/sensitive_records_redacted") {
    Logger logger;
    auto* cap = new CaptureSink;
    logger.addSink(std::unique_ptr<ILogSink>(cap));
    logger.setMinLevel(LogLevel::Trace);

    logger.log(LogLevel::Info, LogCategory::Security, "SECRET_TOKEN=abc123", true);

    SS_REQUIRE(!cap->records.empty());
    SS_CHECK(cap->records[0].sensitive);
    // Sensitive message must be replaced
    SS_CHECK_NE(cap->records[0].message, std::string{"SECRET_TOKEN=abc123"});
}

SS_TEST_CASE("logging/multiple_sinks") {
    Logger logger;
    auto* cap1 = new CaptureSink;
    auto* cap2 = new CaptureSink;
    logger.addSink(std::unique_ptr<ILogSink>(cap1));
    logger.addSink(std::unique_ptr<ILogSink>(cap2));
    logger.setMinLevel(LogLevel::Trace);

    logger.error(LogCategory::Platform, "oops");

    SS_CHECK_EQ(cap1->records.size(), 1u);
    SS_CHECK_EQ(cap2->records.size(), 1u);
}

SS_TEST_CASE("logging/flush_does_not_crash") {
    Logger logger;
    logger.flush();  // With no sinks -- must be safe
}

SS_TEST_CASE("logging/macros_work") {
    Logger logger;
    auto* cap = new CaptureSink;
    logger.addSink(std::unique_ptr<ILogSink>(cap));
    logger.setMinLevel(LogLevel::Trace);

    SS_LOG_INFO(logger, LogCategory::Application, "macro test");

    SS_REQUIRE(!cap->records.empty());
    SS_CHECK(cap->records[0].message == "macro test");
}

int main() { return ss::test::runAll(); }
