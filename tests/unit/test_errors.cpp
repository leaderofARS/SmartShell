// SmartShell -- Unit Tests: Error & Result
#include "../test_framework.hpp"
#include "../../core/errors/error_category.hpp"
#include "../../core/errors/error.hpp"
#include "../../core/errors/result.hpp"

using namespace ss;
using namespace ss::test;

// ---- ErrorCategory::toString ------------------------------------------------
SS_TEST_CASE("error_category/toString_all_values") {
    SS_CHECK(std::string{toString(ErrorCategory::General)}              == "General");
    SS_CHECK(std::string{toString(ErrorCategory::NotFound)}             == "NotFound");
    SS_CHECK(std::string{toString(ErrorCategory::PlatformFailure)}      == "PlatformFailure");
    SS_CHECK(std::string{toString(ErrorCategory::ConfigurationFailure)} == "ConfigurationFailure");
}

// ---- makeError --------------------------------------------------------------
SS_TEST_CASE("error/makeError_fields") {
    auto e = makeError(ErrorCategory::NotFound, "file missing",
                       "readFile", "path=/foo");
    SS_CHECK_EQ(static_cast<int>(e.category), static_cast<int>(ErrorCategory::NotFound));
    SS_CHECK(e.message == "file missing");
    SS_CHECK(e.operation == "readFile");
    SS_CHECK(e.context == "path=/foo");
    SS_CHECK(!e.nativeCode.has_value());
}

SS_TEST_CASE("error/makePlatformError_preserves_native_code") {
    auto e = makePlatformError(2u, "not found", "open", "c:\\missing.txt");
    SS_CHECK(e.nativeCode.has_value());
    SS_CHECK_EQ(*e.nativeCode, 2u);
    SS_CHECK(static_cast<int>(e.category) ==
             static_cast<int>(ErrorCategory::PlatformFailure));
}

SS_TEST_CASE("error/toString_contains_category") {
    auto e = makeError(ErrorCategory::Timeout, "timed out");
    auto s = e.toString();
    SS_CHECK(s.find("Timeout") != std::string::npos);
    SS_CHECK(s.find("timed out") != std::string::npos);
}

// ---- Result<T> --------------------------------------------------------------
SS_TEST_CASE("result/ok_path") {
    Result<int> r{42};
    SS_CHECK(r.isOk());
    SS_CHECK(!r.isError());
    SS_CHECK(static_cast<bool>(r));
    SS_CHECK_EQ(r.value(), 42);
}

SS_TEST_CASE("result/error_path") {
    Result<int> r{makeError(ErrorCategory::General, "boom")};
    SS_CHECK(r.isError());
    SS_CHECK(!r.isOk());
    SS_CHECK(!static_cast<bool>(r));
    SS_CHECK(r.error().message == "boom");
}

SS_TEST_CASE("result/valueOr") {
    Result<int> ok{10};
    Result<int> err{makeError(ErrorCategory::General, "x")};
    SS_CHECK_EQ(ok.valueOr(99), 10);
    SS_CHECK_EQ(err.valueOr(99), 99);
}

SS_TEST_CASE("result/map") {
    Result<int> r{5};
    auto r2 = r.map([](int v) { return v * 2; });
    SS_REQUIRE(r2.isOk());
    SS_CHECK_EQ(r2.value(), 10);
}

// ---- Status -----------------------------------------------------------------
SS_TEST_CASE("status/ok") {
    Status s = Status::ok();
    SS_CHECK(s.isOk());
    SS_CHECK(!s.isError());
}

SS_TEST_CASE("status/err") {
    Status s{makeError(ErrorCategory::IoFailure, "disk error")};
    SS_CHECK(s.isError());
    SS_CHECK(s.error().message == "disk error");
}

SS_TEST_CASE("status/statusOk_helper") {
    auto s = statusOk();
    SS_CHECK(s.isOk());
}

int main() { return ss::test::runAll(); }
