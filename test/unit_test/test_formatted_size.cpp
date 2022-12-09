// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("formatted_size", "[formatted_size]") {
  // Test strategy:
  // * Call emio::formatted_size to determine the total number of characters needed.
  // Expected: The return type, value and the result is correct.

  STATIC_CHECK(emio::formatted_size("{}", 1) == 1U);
  STATIC_CHECK(emio::formatted_size("{} {}", 1, 45) == 4U);
  STATIC_CHECK(emio::formatted_size(emio::runtime{"{}"}, 1, 45) == emio::err::invalid_format);

  emio::result<size_t> res = emio::formatted_size("{}", 1);
  CHECK(res == 1U);
  CHECK(emio::formatted_size("{} {}", 1, 45) == 4U);
  CHECK(emio::formatted_size(emio::runtime{"{}"}, 1, 45) == emio::err::invalid_format);
}

TEST_CASE("vformatted_size", "[formatted_size]") {
  // Test strategy:
  // * Call emio::vformatted_size to determine the total number of characters needed.
  // Expected: The return type, value and the result is correct.

  emio::result<size_t> res = emio::vformatted_size(emio::make_format_args("{}", 1));
  CHECK(res == 1U);
  CHECK(emio::vformatted_size(emio::make_format_args("{} {}", 1, 45)) == 4U);
  CHECK(emio::vformatted_size(emio::make_format_args(emio::runtime{"{}"}, 1, 45)) == emio::err::invalid_format);
}
