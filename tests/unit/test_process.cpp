// SmartShell -- Unit Tests: Process Foundation
#include "../test_framework.hpp"
#include "../../platform/windows/process/process.hpp"
#include <windows.h>

using namespace ss;
using namespace ss::test;
using namespace ss::platform::windows;

SS_TEST_CASE("process/launch_cmd_exit_zero") {
    auto r = ProcessBuilder{}
        .executable(WindowsPath{std::filesystem::path{L"C:\\Windows\\System32\\cmd.exe"}})
        .arguments({"/c", "exit 0"})
        .build();

    SS_REQUIRE(r.isOk());
    auto& proc = r.value();
    SS_CHECK(proc.processId().isValid());
    SS_CHECK(proc.isRunning() || proc.state() == ProcessState::Exited);

    auto exitR = proc.waitForExit(Duration{std::chrono::seconds(10)});
    SS_REQUIRE(exitR.isOk());
    SS_CHECK_EQ(exitR.value(), 0);
}

SS_TEST_CASE("process/launch_cmd_exit_nonzero") {
    auto r = ProcessBuilder{}
        .executable(WindowsPath{std::filesystem::path{L"C:\\Windows\\System32\\cmd.exe"}})
        .arguments({"/c", "exit 42"})
        .build();

    SS_REQUIRE(r.isOk());
    auto exitR = r.value().waitForExit(Duration{std::chrono::seconds(10)});
    SS_REQUIRE(exitR.isOk());
    SS_CHECK_EQ(exitR.value(), 42);
}

SS_TEST_CASE("process/launch_nonexistent_returns_error") {
    auto r = ProcessBuilder{}
        .executable(WindowsPath{std::filesystem::path{L"C:\\nonexistent_app_xyz.exe"}})
        .build();

    SS_CHECK(r.isError());
}

SS_TEST_CASE("process/no_executable_returns_error") {
    auto r = ProcessBuilder{}.build();
    SS_CHECK(r.isError());
}

SS_TEST_CASE("process/try_get_exit_code_after_wait") {
    auto r = ProcessBuilder{}
        .executable(WindowsPath{std::filesystem::path{L"C:\\Windows\\System32\\cmd.exe"}})
        .arguments({"/c", "exit 7"})
        .build();
    SS_REQUIRE(r.isOk());
    auto& proc = r.value();

    proc.waitForExit(Duration{std::chrono::seconds(10)});
    auto code = proc.tryGetExitCode();
    SS_REQUIRE(code.has_value());
    SS_CHECK_EQ(*code, 7);
}

SS_TEST_CASE("process/terminate") {
    // Launch a process that runs indefinitely (ping -t), then terminate it
    auto r = ProcessBuilder{}
        .executable(WindowsPath{std::filesystem::path{L"C:\\Windows\\System32\\ping.exe"}})
        .arguments({"-t", "127.0.0.1"})
        .build();
    SS_REQUIRE(r.isOk());

    auto st = r.value().terminate();
    SS_CHECK_NO_ERROR(st);
    SS_CHECK(r.value().state() == ProcessState::Terminated);
}

int main() { return ss::test::runAll(); }
