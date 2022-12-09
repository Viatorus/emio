// Unit under test.
#include <emio/detail/conversion.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>

// TODO: More conversion tests

TEST_CASE("char_to_digit") {
  // Test strategy:
  // * Call char_to_digits with possible inputs and edge cases.
  // Expected: Correct digit or an error is returned.

  using emio::detail::char_to_digit;

  SECTION("0-9") {
    const int base = GENERATE(range(2, 36 + 1));
    const int val = GENERATE(range(0, 9 + 1));
    const char c = static_cast<char>('0' + val);

    if (val < base) {
      CHECK(char_to_digit(c, base) == val);
    } else {
      CHECK(char_to_digit(c, base) == std::nullopt);
    }
  }
  SECTION("A-Z") {
    const int base = GENERATE(range(2, 36 + 1));
    const int val = GENERATE(range(0, 26 + 1)) + 10;
    const char c = static_cast<char>('A' + val - 10);

    if (val < base) {
      CHECK(char_to_digit(c, base) == val);
    } else {
      CHECK(char_to_digit(c, base) == std::nullopt);
    }
  }
  SECTION("a-z") {
    const int base = GENERATE(range(2, 36 + 1));
    const int val = GENERATE(range(0, 26 + 1)) + 10;
    const char c = static_cast<char>('a' + val - 10);

    if (val < base) {
      CHECK(char_to_digit(c, base) == val);
    } else {
      CHECK(char_to_digit(c, base) == std::nullopt);
    }
  }
  SECTION("some edge cases") {
    CHECK(!char_to_digit('\x09', 2));
    CHECK(char_to_digit('0', 2) == 0);
    CHECK(char_to_digit('1', 2) == 1);
    CHECK(!char_to_digit('2', 2));

    CHECK(!char_to_digit('\x01', 2));
    CHECK(char_to_digit('0', 8) == 0);
    CHECK(char_to_digit('7', 8) == 7);
    CHECK(!char_to_digit('8', 8));

    CHECK(!char_to_digit('\x10', 10));
    CHECK(!char_to_digit('/', 10));
    CHECK(!char_to_digit(':', 10));
    CHECK(char_to_digit('9', 10) == 9);
    CHECK(!char_to_digit('A', 10));
    CHECK(!char_to_digit('a', 10));

    CHECK(char_to_digit('F', 16) == 15);
    CHECK(char_to_digit('f', 16) == 15);
    CHECK(!char_to_digit('G', 16));
    CHECK(!char_to_digit('g', 16));

    CHECK(char_to_digit('V', 32) == 31);
    CHECK(!char_to_digit('W', 32));
    CHECK(char_to_digit('v', 32) == 31);
    CHECK(!char_to_digit('w', 32));

    CHECK(char_to_digit('Z', 36) == 35);
    CHECK(!char_to_digit('[', 36));
    CHECK(char_to_digit('z', 36) == 35);
    CHECK(!char_to_digit('{', 36));

    CHECK(!char_to_digit(std::numeric_limits<char>::min(), 2));
    CHECK(!char_to_digit(std::numeric_limits<char>::max(), 2));
    CHECK(!char_to_digit(std::numeric_limits<char>::min(), 36));
    CHECK(!char_to_digit(std::numeric_limits<char>::max(), 36));
  }
}
