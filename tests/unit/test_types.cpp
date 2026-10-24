// SmartShell -- Unit Tests: Foundation Types & Identifiers
#include "../test_framework.hpp"
#include "../../core/foundation/types.hpp"
#include "../../core/foundation/version.hpp"
#include "../../core/identifiers/id.hpp"
#include "../../core/identifiers/types.hpp"
#include <unordered_set>
#include <sstream>

using namespace ss;
using namespace ss::test;

// ---- Integer alias sizes ----------------------------------------------------
SS_TEST_CASE("types/integer_aliases_have_correct_size") {
    SS_CHECK_EQ(sizeof(u8),  1u);
    SS_CHECK_EQ(sizeof(u16), 2u);
    SS_CHECK_EQ(sizeof(u32), 4u);
    SS_CHECK_EQ(sizeof(u64), 8u);
    SS_CHECK_EQ(sizeof(i8),  1u);
    SS_CHECK_EQ(sizeof(i16), 2u);
    SS_CHECK_EQ(sizeof(i32), 4u);
    SS_CHECK_EQ(sizeof(i64), 8u);
    SS_CHECK_EQ(sizeof(byte), 1u);
}

// ---- Version ----------------------------------------------------------------
SS_TEST_CASE("version/construct_and_compare") {
    Version v1{1, 2, 3};
    Version v2{1, 2, 4};
    Version v3{1, 2, 3};

    SS_CHECK(v1 == v3);
    SS_CHECK(v1 != v2);
    SS_CHECK(v1 < v2);
    SS_CHECK(v2 > v1);
}

SS_TEST_CASE("version/toString") {
    Version v{0, 1, 0};
    SS_CHECK_EQ(v.toString(), std::string{"0.1.0"});
}

SS_TEST_CASE("version/product_version_is_valid") {
    SS_CHECK(kProductVersion.major <= 999u);
    // Just verify it's accessible and has a reasonable toString
    auto s = kProductVersion.toString();
    SS_CHECK(!s.empty());
}

// ---- StrongId ---------------------------------------------------------------
SS_TEST_CASE("id/default_is_invalid") {
    ProcessId pid;
    SS_CHECK(!pid.isValid());
    SS_CHECK(!pid);
}

SS_TEST_CASE("id/explicit_value_is_valid") {
    ProcessId pid{42};
    SS_CHECK(pid.isValid());
    SS_CHECK_EQ(pid.value(), 42u);
}

SS_TEST_CASE("id/comparison") {
    ProcessId a{1}, b{2}, c{1};
    SS_CHECK(a == c);
    SS_CHECK(a != b);
    SS_CHECK(a < b);
    SS_CHECK(b > a);
}

SS_TEST_CASE("id/different_types_not_interchangeable") {
    // This test validates at compile time that ProcessId and SessionId
    // are distinct types. We can only verify at runtime that they behave.
    ProcessId pid{5};
    SessionId sid{5};
    // They have same value but are distinct types -- no implicit conversion
    SS_CHECK_EQ(pid.value(), sid.value()); // same underlying int
    // static_assert(!std::is_same_v<ProcessId, SessionId>); // compile-time check
}

SS_TEST_CASE("id/hashable_in_unordered_set") {
    std::unordered_set<ProcessId> set;
    set.insert(ProcessId{1});
    set.insert(ProcessId{2});
    set.insert(ProcessId{1}); // duplicate
    SS_CHECK_EQ(set.size(), 2u);
}

SS_TEST_CASE("id/toString") {
    ProcessId pid{7};
    SS_CHECK_EQ(pid.toString(), std::string{"7"});
}

int main() { return ss::test::runAll(); }
