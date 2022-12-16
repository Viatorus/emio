// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("emio::format", "[format]") {
  // Test strategy:
  // * Call emio::format.
  // Expected: The return types, values and the format results are correct.

  SECTION("success") {
    std::string res = emio::format("{}", 42);
    CHECK(res == "42");
  }
  SECTION("invalid_format") {
    emio::result<std::string> res = emio::format(emio::runtime{"{"}, 42);
    CHECK(res == emio::err::invalid_format);
  }
}

TEST_CASE("emio::vformat", "[format]") {
  // Test strategy:
  // * Call emio::vformat.
  // Expected: The return types, values and the format results are correct.

  SECTION("success") {
    emio::result<std::string> res = emio::vformat(emio::make_format_args("{}", 42));
    CHECK(res == "42");
  }
  SECTION("invalid_format") {
    emio::result<std::string> res = emio::vformat(emio::make_format_args(emio::runtime{"{"}, 42));
    CHECK(res == emio::err::invalid_format);
  }
}
