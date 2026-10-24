// SmartShell -- Unit Tests: Environment
#include "../test_framework.hpp"
#include "../../platform/windows/environment/environment.hpp"
#include <string>

using namespace ss;
using namespace ss::test;
using namespace ss::platform::windows;

// Use a unique prefix to avoid contaminating real environment
static constexpr Utf8View kTestPrefix = "SS_TEST_ENV_VAR_";

SS_TEST_CASE("environment/set_and_get_variable") {
    Utf8String key = Utf8String{kTestPrefix} + "BASIC";
    Utf8String val = "hello_value";

    auto st = Environment::setVariable(key, val);
    SS_CHECK_NO_ERROR(st);

    auto got = Environment::getVariable(key);
    SS_REQUIRE(got.has_value());
    SS_CHECK_EQ(*got, val);

    Environment::removeVariable(key);
}

SS_TEST_CASE("environment/get_missing_returns_nullopt") {
    auto got = Environment::getVariable("SS_THIS_DOES_NOT_EXIST_XYZ123");
    SS_CHECK(!got.has_value());
}

SS_TEST_CASE("environment/remove_variable") {
    Utf8String key = Utf8String{kTestPrefix} + "REMOVE";
    Environment::setVariable(key, "to_remove");

    auto st = Environment::removeVariable(key);
    SS_CHECK_NO_ERROR(st);
    SS_CHECK(!Environment::getVariable(key).has_value());
}

SS_TEST_CASE("environment/remove_nonexistent_is_ok") {
    // Removing a variable that doesn't exist should succeed silently
    auto st = Environment::removeVariable("SS_NONEXISTENT_XYZ999");
    SS_CHECK_NO_ERROR(st);
}

SS_TEST_CASE("environment/get_all_contains_path") {
    auto r = Environment::getAll();
    SS_REQUIRE(r.isOk());
    // PATH must be present in any Windows process
    SS_CHECK(r.value().count("PATH") > 0 || r.value().count("Path") > 0);
}

SS_TEST_CASE("environment/unicode_value") {
    Utf8String key = Utf8String{kTestPrefix} + "UNICODE";
    // Japanese: テスト
    Utf8String val = "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88";

    auto st = Environment::setVariable(key, val);
    SS_CHECK_NO_ERROR(st);

    auto got = Environment::getVariable(key);
    SS_REQUIRE(got.has_value());
    SS_CHECK_EQ(*got, val);

    Environment::removeVariable(key);
}

SS_TEST_CASE("environment/get_current_directory") {
    auto r = Environment::getCurrentDirectory();
    SS_REQUIRE(r.isOk());
    SS_CHECK(!r.value().empty());
}

SS_TEST_CASE("environment/environment_block_build") {
    auto r = EnvironmentBlock::fromCurrentProcess();
    SS_REQUIRE(r.isOk());

    r.value().set("SS_BLOCK_TEST", "42");
    r.value().remove("PATH");  // safe to remove from block (not from process)

    auto block = r.value().build();
    SS_CHECK(!block.empty());
    SS_CHECK(block.back() == L'\0');  // double-null terminated
}

int main() { return ss::test::runAll(); }
