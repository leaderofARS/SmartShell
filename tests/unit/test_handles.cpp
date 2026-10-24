// SmartShell -- Unit Tests: Windows HANDLE Ownership
#include "../test_framework.hpp"
#include "../../platform/windows/handles/handle.hpp"
#include <windows.h>

using namespace ss;
using namespace ss::test;
using namespace ss::platform::windows;

SS_TEST_CASE("handle/default_is_invalid") {
    KernelHandle h;
    SS_CHECK(!h.valid());
    SS_CHECK(!static_cast<bool>(h));
    SS_CHECK(h.get() == INVALID_HANDLE_VALUE);
}

SS_TEST_CASE("handle/null_default_is_invalid") {
    NullHandle h;
    SS_CHECK(!h.valid());
    SS_CHECK(h.get() == nullptr);
}

SS_TEST_CASE("handle/valid_event_handle") {
    HANDLE raw = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    SS_REQUIRE(raw != nullptr);
    NullHandle h{raw};
    SS_CHECK(h.valid());
    SS_CHECK(h.get() == raw);
    // h destroyed here -- CloseHandle called automatically
}

SS_TEST_CASE("handle/move_transfers_ownership") {
    HANDLE raw = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    SS_REQUIRE(raw != nullptr);
    NullHandle h1{raw};
    NullHandle h2 = std::move(h1);
    SS_CHECK(!h1.valid());   // h1 released
    SS_CHECK(h2.valid());    // h2 owns it
    SS_CHECK_EQ(h2.get(), raw);
}

SS_TEST_CASE("handle/reset_closes_old_and_takes_new") {
    HANDLE raw1 = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    HANDLE raw2 = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    SS_REQUIRE(raw1 != nullptr && raw2 != nullptr);

    NullHandle h{raw1};
    h.reset(raw2);   // raw1 closed here
    SS_CHECK(h.get() == raw2);
}

SS_TEST_CASE("handle/release_gives_up_ownership") {
    HANDLE raw = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    SS_REQUIRE(raw != nullptr);
    NullHandle h{raw};
    HANDLE released = h.release();
    SS_CHECK(!h.valid());
    SS_CHECK_EQ(released, raw);
    ::CloseHandle(released);  // manual close since we released
}

SS_TEST_CASE("handle/kernel_handle_with_file") {
    // Create a temp file to get a real INVALID_HANDLE_VALUE-using handle
    wchar_t tmpPath[MAX_PATH + 1];
    ::GetTempPathW(MAX_PATH, tmpPath);
    wchar_t tmpFile[MAX_PATH + 1];
    ::GetTempFileNameW(tmpPath, L"ss_", 0, tmpFile);

    HANDLE raw = ::CreateFileW(tmpFile, GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    SS_REQUIRE(raw != INVALID_HANDLE_VALUE);

    {
        KernelHandle h{raw};
        SS_CHECK(h.valid());
    }  // h destroyed, CloseHandle called

    ::DeleteFileW(tmpFile);
}

int main() { return ss::test::runAll(); }
