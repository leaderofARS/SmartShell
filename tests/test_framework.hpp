// SmartShell -- Minimal Test Framework (no external dependencies)
//
// Usage:
//   SS_TEST_CASE("MyTest") { SS_CHECK(1 + 1 == 2); }
//   int main() { return ss::test::runAll(); }
#pragma once

#include <cstdio>
#include <cstring>
#include <functional>
#include <vector>
#include <string>
#include <sstream>

namespace ss::test {

// ---- Test registration infrastructure ---------------------------------------

struct TestCase {
    const char*            name;
    const char*            file;
    int                    line;
    std::function<void()>  fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

struct Registrar {
    Registrar(const char* name, const char* file, int line, std::function<void()> fn) {
        registry().push_back({name, file, line, std::move(fn)});
    }
};

// Per-test failure tracking
inline int  g_checks   = 0;
inline int  g_failures = 0;
inline bool g_fatal    = false;

// ---- Check macros -----------------------------------------------------------

inline void checkFail(const char* expr, const char* file, int line,
                      const char* extra = nullptr) {
    ++g_failures;
    std::fprintf(stderr, "  FAIL  %s:%d: %s", file, line, expr);
    if (extra && extra[0]) std::fprintf(stderr, " -- %s", extra);
    std::fprintf(stderr, "\n");
}

#define SS_CHECK(expr) \
    do { \
        ++::ss::test::g_checks; \
        if (!(expr)) { \
            ::ss::test::checkFail(#expr, __FILE__, __LINE__); \
        } \
    } while(false)

#define SS_REQUIRE(expr) \
    do { \
        ++::ss::test::g_checks; \
        if (!(expr)) { \
            ::ss::test::checkFail(#expr, __FILE__, __LINE__, "FATAL"); \
            ::ss::test::g_fatal = true; \
            return; \
        } \
    } while(false)

#define SS_CHECK_EQ(a, b) \
    do { \
        ++::ss::test::g_checks; \
        if (!((a) == (b))) { \
            std::ostringstream _ss_oss; \
            _ss_oss << #a " == " #b; \
            ::ss::test::checkFail(_ss_oss.str().c_str(), __FILE__, __LINE__); \
        } \
    } while(false)

#define SS_CHECK_NE(a, b) \
    do { \
        ++::ss::test::g_checks; \
        if ((a) == (b)) { \
            ::ss::test::checkFail(#a " != " #b, __FILE__, __LINE__); \
        } \
    } while(false)

#define SS_CHECK_NO_ERROR(result) \
    do { \
        ++::ss::test::g_checks; \
        auto& _r = (result); \
        if (_r.isError()) { \
            std::string _msg = "Expected ok but got error: " + _r.error().toString(); \
            ::ss::test::checkFail(_msg.c_str(), __FILE__, __LINE__); \
        } \
    } while(false)

#define SS_CHECK_IS_ERROR(result) \
    do { \
        ++::ss::test::g_checks; \
        if ((result).isOk()) { \
            ::ss::test::checkFail("Expected error but got ok: " #result, \
                                  __FILE__, __LINE__); \
        } \
    } while(false)

// ---- Test case registration -------------------------------------------------

#define SS_TEST_CONCAT_IMPL(a, b) a##b
#define SS_TEST_CONCAT(a, b) SS_TEST_CONCAT_IMPL(a, b)

#define SS_TEST_CASE(name_) \
    static void SS_TEST_CONCAT(_ss_test_body_, __LINE__)(); \
    static ::ss::test::Registrar SS_TEST_CONCAT(_ss_reg_, __LINE__){ \
        name_, __FILE__, __LINE__, SS_TEST_CONCAT(_ss_test_body_, __LINE__)}; \
    static void SS_TEST_CONCAT(_ss_test_body_, __LINE__)()

// ---- Test runner ------------------------------------------------------------

inline int runAll() {
    int totalChecks   = 0;
    int totalFailures = 0;
    int totalTests    = 0;
    int failedTests   = 0;

    std::printf("=== SmartShell Test Runner ===\n");

    for (auto& tc : registry()) {
        g_checks   = 0;
        g_failures = 0;
        g_fatal    = false;

        std::printf("[ RUN  ] %s\n", tc.name);
        try {
            tc.fn();
        } catch (const std::exception& ex) {
            ++g_failures;
            std::fprintf(stderr, "  EXCEPTION: %s\n", ex.what());
        } catch (...) {
            ++g_failures;
            std::fprintf(stderr, "  UNKNOWN EXCEPTION\n");
        }

        totalChecks   += g_checks;
        totalFailures += g_failures;
        ++totalTests;

        if (g_failures == 0) {
            std::printf("[ PASS ] %s (%d checks)\n", tc.name, g_checks);
        } else {
            std::printf("[ FAIL ] %s (%d/%d checks failed)\n",
                        tc.name, g_failures, g_checks);
            ++failedTests;
        }
    }

    std::printf("\n=== Results: %d/%d tests passed, %d/%d checks passed ===\n",
                totalTests - failedTests, totalTests,
                totalChecks - totalFailures, totalChecks);

    return failedTests == 0 ? 0 : 1;
}

} // namespace ss::test
