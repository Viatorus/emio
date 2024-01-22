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

namespace {

struct throws_on_move {
  throws_on_move() = default;
  throws_on_move(throws_on_move&&) {
    throw std::runtime_error{"As expected..."};
  }
};

std::string_view format_as(const throws_on_move&) {
  throw std::logic_error{"Shouldn't be called."};
}

}  // namespace

TEST_CASE("std::variant") {
  STATIC_CHECK(emio::is_formattable_v<std::string>);

  std::variant<std::monostate, int, double, char, nullptr_t, std::string> v{};
  CHECK(emio::format("{}", v) == "variant(monostate)");
  v = 42;
  CHECK(emio::format("{}", v) == "variant(42)");
  v = 4.2;
  CHECK(emio::format("{}", v) == "variant(4.2)");
  v = 'x';
  CHECK(emio::format("{}", v) == "variant('x')");
  v = nullptr;
  CHECK(emio::format("{}", v) == "variant(0x0)");
  v = "abc";
  CHECK(emio::format("{}", v) == "variant(\"abc\")");

  SECTION("valueless by exception") {
    std::variant<std::monostate, throws_on_move> v2;
    try {
      throws_on_move thrower;
      v2.emplace<throws_on_move>(std::move(thrower));
    } catch (const std::runtime_error&) {
    }
    CHECK(emio::format("{}", v2) == "variant(valueless by exception)");
  }
}
