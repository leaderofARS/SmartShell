// SmartShell -- Unit Tests: Configuration
#include "../test_framework.hpp"
#include "../../core/configuration/value.hpp"
#include "../../core/configuration/path.hpp"
#include "../../core/configuration/node.hpp"
#include "../../core/configuration/store.hpp"

using namespace ss;
using namespace ss::test;

// ---- ConfigPath -------------------------------------------------------------
SS_TEST_CASE("config/path_parse_simple") {
    auto p = ConfigPath::parse("key");
    SS_CHECK_EQ(p.size(), 1u);
    SS_CHECK_EQ(p.parts()[0], std::string{"key"});
}

SS_TEST_CASE("config/path_parse_dotted") {
    auto p = ConfigPath::parse("terminal.font.size");
    SS_CHECK_EQ(p.size(), 3u);
    SS_CHECK_EQ(p.parts()[0], std::string{"terminal"});
    SS_CHECK_EQ(p.parts()[1], std::string{"font"});
    SS_CHECK_EQ(p.parts()[2], std::string{"size"});
}

SS_TEST_CASE("config/path_toString") {
    auto p = ConfigPath::parse("a.b.c");
    SS_CHECK_EQ(p.toString(), std::string{"a.b.c"});
}

SS_TEST_CASE("config/path_empty") {
    auto p = ConfigPath::parse("");
    SS_CHECK(p.empty());
}

SS_TEST_CASE("config/path_head_and_tail") {
    auto p = ConfigPath::parse("x.y.z");
    SS_CHECK_EQ(p.head(), std::string{"x"});
    auto t = p.tail();
    SS_CHECK_EQ(t.size(), 2u);
    SS_CHECK_EQ(t.head(), std::string{"y"});
}

// ---- ConfigValue ------------------------------------------------------------
SS_TEST_CASE("config/value_null") {
    ConfigValue v;
    SS_CHECK(v.isNull());
    SS_CHECK(v.type() == ConfigValueType::Null);
}

SS_TEST_CASE("config/value_bool") {
    ConfigValue v{true};
    SS_CHECK(v.isBool());
    SS_CHECK(v.asBool() == true);
}

SS_TEST_CASE("config/value_integer") {
    ConfigValue v{i64{42}};
    SS_CHECK(v.isInteger());
    SS_CHECK_EQ(v.asInteger(), i64{42});
}

SS_TEST_CASE("config/value_string") {
    ConfigValue v{Utf8String{"hello"}};
    SS_CHECK(v.isString());
    SS_CHECK_EQ(v.asString(), std::string{"hello"});
}

SS_TEST_CASE("config/value_equality") {
    ConfigValue a{42}, b{42}, c{43};
    SS_CHECK(a == b);
    SS_CHECK(!(a == c));
}

// ---- ConfigNode -------------------------------------------------------------
SS_TEST_CASE("config/node_set_and_get") {
    ConfigNode node;
    auto st = node.set(ConfigPath::parse("font.size"), ConfigValue{i64{14}});
    SS_CHECK_NO_ERROR(st);

    auto v = node.get(ConfigPath::parse("font.size"));
    SS_REQUIRE(v.has_value());
    SS_CHECK(v->isInteger());
    SS_CHECK_EQ(v->asInteger(), i64{14});
}

SS_TEST_CASE("config/node_contains") {
    ConfigNode node;
    node.set(ConfigPath::parse("a.b"), ConfigValue{true});
    SS_CHECK(node.contains(ConfigPath::parse("a.b")));
    SS_CHECK(!node.contains(ConfigPath::parse("a.c")));
}

SS_TEST_CASE("config/node_missing_returns_nullopt") {
    ConfigNode node;
    auto v = node.get(ConfigPath::parse("nope"));
    SS_CHECK(!v.has_value());
}

SS_TEST_CASE("config/node_remove") {
    ConfigNode node;
    node.set(ConfigPath::parse("x"), ConfigValue{i64{1}});
    SS_CHECK(node.contains(ConfigPath::parse("x")));
    auto st = node.remove(ConfigPath::parse("x"));
    SS_CHECK_NO_ERROR(st);
    SS_CHECK(!node.contains(ConfigPath::parse("x")));
}

// ---- ConfigStore ------------------------------------------------------------
SS_TEST_CASE("config/store_layering_defaults_overridden_by_user") {
    ConfigStore store;
    store.set(ConfigLayer::Defaults, "tab.size", ConfigValue{i64{4}});
    store.set(ConfigLayer::User,     "tab.size", ConfigValue{i64{2}});

    auto v = store.getEffective("tab.size");
    SS_REQUIRE(v.has_value());
    SS_CHECK_EQ(v->asInteger(), i64{2});  // User wins over defaults
}

SS_TEST_CASE("config/store_fallback_to_defaults") {
    ConfigStore store;
    store.set(ConfigLayer::Defaults, "cursor.style", ConfigValue{Utf8String{"block"}});

    auto v = store.getEffective("cursor.style");
    SS_REQUIRE(v.has_value());
    SS_CHECK_EQ(v->asString(), std::string{"block"});
}

SS_TEST_CASE("config/store_missing_returns_nullopt") {
    ConfigStore store;
    auto v = store.getEffective("nonexistent.key");
    SS_CHECK(!v.has_value());
}

SS_TEST_CASE("config/store_runtime_highest_priority") {
    ConfigStore store;
    store.set(ConfigLayer::Defaults, "x", ConfigValue{i64{1}});
    store.set(ConfigLayer::User,     "x", ConfigValue{i64{2}});
    store.set(ConfigLayer::Runtime,  "x", ConfigValue{i64{99}});

    auto v = store.getEffective("x");
    SS_REQUIRE(v.has_value());
    SS_CHECK_EQ(v->asInteger(), i64{99});
}

int main() { return ss::test::runAll(); }
