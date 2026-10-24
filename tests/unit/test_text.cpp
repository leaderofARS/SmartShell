// SmartShell -- Unit Tests: Text Encoding
#include "../test_framework.hpp"
#include "../../core/text/encoding.hpp"

using namespace ss;
using namespace ss::test;

SS_TEST_CASE("text/toUtf16_empty") {
    auto r = toUtf16("");
    SS_REQUIRE(r.isOk());
    SS_CHECK(r.value().empty());
}

SS_TEST_CASE("text/toUtf8_empty") {
    auto r = toUtf8(L"");
    SS_REQUIRE(r.isOk());
    SS_CHECK(r.value().empty());
}

SS_TEST_CASE("text/ascii_roundtrip") {
    Utf8View original = "Hello, SmartShell!";
    auto wide = toUtf16(original);
    SS_REQUIRE(wide.isOk());
    auto back = toUtf8(wide.value());
    SS_REQUIRE(back.isOk());
    SS_CHECK_EQ(back.value(), std::string{original});
}

SS_TEST_CASE("text/bmp_unicode_roundtrip") {
    // Greek letters: "αβγ" (U+03B1 U+03B2 U+03B3)
    Utf8String original = "\xCE\xB1\xCE\xB2\xCE\xB3";
    auto wide = toUtf16(original);
    SS_REQUIRE(wide.isOk());
    auto back = toUtf8(wide.value());
    SS_REQUIRE(back.isOk());
    SS_CHECK_EQ(back.value(), original);
}

SS_TEST_CASE("text/supplementary_plane_roundtrip") {
    // Mathematical Fraktur A: U+1D504 (encoded as UTF-8 surrogate pair in UTF-16)
    Utf8String original = "\xF0\x9D\x94\x84";
    auto wide = toUtf16(original);
    SS_REQUIRE(wide.isOk());
    auto back = toUtf8(wide.value());
    SS_REQUIRE(back.isOk());
    SS_CHECK_EQ(back.value(), original);
}

SS_TEST_CASE("text/invalid_utf8_returns_error") {
    // 0xFF is not valid in any UTF-8 sequence
    Utf8String bad = "\xFF\xFE invalid";
    auto r = toUtf16(bad);
    SS_CHECK(r.isError());
}

SS_TEST_CASE("text/path_with_unicode") {
    // Japanese: "テスト" (te-su-to)
    Utf8String japanese = "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88";
    auto wide = toUtf16(japanese);
    SS_REQUIRE(wide.isOk());
    SS_CHECK_EQ(wide.value().size(), 3u);  // 3 BMP chars
    auto back = toUtf8(wide.value());
    SS_REQUIRE(back.isOk());
    SS_CHECK_EQ(back.value(), japanese);
}

int main() { return ss::test::runAll(); }
