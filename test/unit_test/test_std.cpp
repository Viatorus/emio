// Unit under test.
#include <emio/std.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <emio/format.hpp>
#include <emio/ranges.hpp>

struct unformattable {};

TEST_CASE("std::optional") {
  CHECK(emio::format("{}", std::optional<int>{}) == "none");
  CHECK(emio::format("{:x}", std::optional<int>{42}) == "optional(2a)");
  CHECK(emio::format("{:x}", std::optional<int>{42}) == "optional(2a)");
  CHECK(emio::format("{}", std::optional{std::vector{'h', 'e', 'l', 'l', 'o'}}) ==
        "optional(['h', 'e', 'l', 'l', 'o'])");
  CHECK(emio::format("{::d}", std::optional{std::vector{'h', 'e', 'l', 'l', 'o'}}) ==
        "optional([104, 101, 108, 108, 111])");

  STATIC_CHECK(emio::is_formattable_v<std::optional<int>>);
  STATIC_CHECK_FALSE(emio::is_formattable_v<std::optional<unformattable>>);

  // TODO debug
}

TEST_CASE("std::exception") {
  CHECK(emio::format("{}", std::exception{}) == "std::exception");
  CHECK(emio::format("{}", std::runtime_error{"hello"}) == "hello");
}

TEST_CASE("std::filesystem::path") {
  using std::filesystem::path;

  CHECK(emio::format("{}", path{}) == "");
  CHECK(emio::format("{}", path{"/abc/dev"}) == "/abc/dev");
  CHECK(emio::format("{:x>11}", path{"/abc/dev"}) == "xxx/abc/dev");
  CHECK(emio::format("{:x<11?}", path{"/abc/dev"}) == "\"/abc/dev\"x");
}

