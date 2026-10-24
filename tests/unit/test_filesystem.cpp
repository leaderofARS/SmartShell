// SmartShell -- Unit Tests: Filesystem
#include "../test_framework.hpp"
#include "../../platform/windows/filesystem/path.hpp"
#include "../../platform/windows/filesystem/filesystem.hpp"
#include <string>

using namespace ss;
using namespace ss::test;
using namespace ss::platform::windows;
using namespace ss::platform::windows::fs;

SS_TEST_CASE("filesystem/create_temp_directory") {
    auto r = createTempDirectory();
    SS_REQUIRE(r.isOk());
    auto& dir = r.value();
    SS_CHECK(isDirectory(dir));
    // Cleanup
    auto st = deleteDirectory(dir, true);
    SS_CHECK_NO_ERROR(st);
}

SS_TEST_CASE("filesystem/write_and_read_text_file") {
    auto tmpDir = createTempDirectory();
    SS_REQUIRE(tmpDir.isOk());
    auto dir = tmpDir.value();

    auto filePath = dir / WindowsPath{std::filesystem::path{L"test.txt"}};
    Utf8String content = "Hello, SmartShell!\nSecond line.\n";

    auto wst = writeTextFile(filePath, content);
    SS_CHECK_NO_ERROR(wst);
    SS_CHECK(isFile(filePath));

    auto rr = readTextFile(filePath);
    SS_REQUIRE(rr.isOk());
    SS_CHECK_EQ(rr.value(), content);

    deleteDirectory(dir, true);
}

SS_TEST_CASE("filesystem/rename_file") {
    auto tmpDir = createTempDirectory();
    SS_REQUIRE(tmpDir.isOk());
    auto dir = tmpDir.value();

    auto src = dir / WindowsPath{std::filesystem::path{L"src.txt"}};
    auto dst = dir / WindowsPath{std::filesystem::path{L"dst.txt"}};

    writeTextFile(src, "data");
    SS_CHECK(exists(src));

    auto st = renameFile(src, dst);
    SS_CHECK_NO_ERROR(st);
    SS_CHECK(!exists(src));
    SS_CHECK(exists(dst));

    deleteDirectory(dir, true);
}

SS_TEST_CASE("filesystem/delete_file") {
    auto tmpDir = createTempDirectory();
    SS_REQUIRE(tmpDir.isOk());
    auto dir = tmpDir.value();
    auto file = dir / WindowsPath{std::filesystem::path{L"del.txt"}};

    writeTextFile(file, "x");
    SS_CHECK(exists(file));

    auto st = deleteFile(file);
    SS_CHECK_NO_ERROR(st);
    SS_CHECK(!exists(file));

    deleteDirectory(dir, true);
}

SS_TEST_CASE("filesystem/exists_returns_false_for_missing") {
    auto p = WindowsPath{std::filesystem::path{L"C:\\nonexistent_ss_test_xyz123"}};
    SS_CHECK(!exists(p));
}

SS_TEST_CASE("filesystem/create_directory") {
    auto tmpDir = createTempDirectory();
    SS_REQUIRE(tmpDir.isOk());
    auto dir = tmpDir.value();
    auto sub = dir / WindowsPath{std::filesystem::path{L"subdir"}};

    auto st = createDirectory(sub);
    SS_CHECK_NO_ERROR(st);
    SS_CHECK(isDirectory(sub));

    deleteDirectory(dir, true);
}

SS_TEST_CASE("filesystem/list_directory") {
    auto tmpDir = createTempDirectory();
    SS_REQUIRE(tmpDir.isOk());
    auto dir = tmpDir.value();

    writeTextFile(dir / WindowsPath{std::filesystem::path{L"a.txt"}}, "a");
    writeTextFile(dir / WindowsPath{std::filesystem::path{L"b.txt"}}, "b");

    auto r = listDirectory(dir);
    SS_REQUIRE(r.isOk());
    SS_CHECK(r.value().size() >= 2u);

    deleteDirectory(dir, true);
}

SS_TEST_CASE("filesystem/atomic_write") {
    auto tmpDir = createTempDirectory();
    SS_REQUIRE(tmpDir.isOk());
    auto dir = tmpDir.value();
    auto target = dir / WindowsPath{std::filesystem::path{L"atomic.txt"}};

    Utf8String content = "atomic content";
    std::vector<byte> data(content.begin(), content.end());
    auto st = atomicWriteFile(target, data);
    SS_CHECK_NO_ERROR(st);

    auto rr = readTextFile(target);
    SS_REQUIRE(rr.isOk());
    SS_CHECK_EQ(rr.value(), content);

    deleteDirectory(dir, true);
}

SS_TEST_CASE("filesystem/unicode_path") {
    auto tmpDir = createTempDirectory();
    SS_REQUIRE(tmpDir.isOk());
    auto dir = tmpDir.value();
    // Japanese filename: テスト.txt
    auto file = dir / WindowsPath{std::filesystem::path{L"\u30C6\u30B9\u30C8.txt"}};

    auto st = writeTextFile(file, "unicode path test");
    SS_CHECK_NO_ERROR(st);
    SS_CHECK(exists(file));

    deleteDirectory(dir, true);
}

int main() { return ss::test::runAll(); }
