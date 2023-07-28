// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

namespace foo {

enum class bar {};

constexpr auto format_as(const bar& w) {
  return static_cast<std::underlying_type_t<bar>>(w);
}

}  // namespace foo

TEST_CASE("format_as", "[formatter]") {
  // Test strategy:
  // * Format a custom type with a format_as.
  // Expected: The formatting works.

  SECTION("compile-time") {
    SECTION("simple") {
      constexpr bool success = [] {
        emio::static_buffer<10> buf{};

        static_cast<void>(emio::format_to(buf, "{}", foo::bar{42}).value());
        return buf.view() == "42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("complex") {
      constexpr bool success = [] {
        emio::static_buffer<10> buf{};

        static_cast<void>(emio::format_to(buf, "{:x<4x}", foo::bar{42}).value());
        return buf.view() == "2axx";
      }();
      STATIC_CHECK(success);
    }
  }

  CHECK(emio::format("{}", foo::bar{42}) == "42");
  CHECK(emio::format("{:x<4x}", foo::bar{42}) == "2axx");
}
