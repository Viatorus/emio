// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;
using namespace std::string_literals;

namespace {

constexpr std::string_view format_str{"abc {} {} {}"};
constexpr emio::format_string<int, char, std::string_view> precompiled_format_str{format_str};
constexpr emio::valid_format_string<int, char, std::string_view> precompiled_validated_format_str{
    precompiled_format_str.as_valid().value()};
constexpr std::string_view expected_str{"abc 42 x hello"};

}  // namespace

TEST_CASE("allowed types for format string", "[format_string]") {
  // Test strategy:
  // * Call emio::format, make_format_args and formatted_size with different format string types.
  // Expected: Constexpr, runtime and precompiled format string work, the results are the same.

  SECTION("format") {
    std::string str = emio::format(format_str, 42, 'x', "hello"sv);
    CHECK(str == expected_str);

    emio::result<std::string> res = emio::format(emio::runtime{format_str}, 42, 'x', "hello"sv);
    CHECK(res == expected_str);

    res = emio::format(precompiled_format_str, 42, 'x', "hello"sv);
    CHECK(res == expected_str);

    str = emio::format(precompiled_validated_format_str, 42, 'x', "hello"sv);
    CHECK(str == expected_str);
  }

  SECTION("make_format_args") {
    {
      emio::format_args&& args = emio::make_format_args(format_str, 42, 'x', "hello"sv);
      static_cast<void>(args);
    }

    {
      emio::format_args&& args = emio::make_format_args(emio::runtime{format_str}, 42, 'x', "hello"sv);
      static_cast<void>(args);
    }
    {
      emio::format_args&& args = emio::make_format_args(precompiled_format_str, 42, 'x', "hello"sv);
      static_cast<void>(args);
    }
  }

  SECTION("formatted_size") {
    {
      constexpr size_t res = emio::formatted_size(format_str, 42, 'x', "hello"sv);
      STATIC_CHECK(res == 14UL);
    }

    {
      constexpr emio::result<size_t> res = emio::formatted_size(emio::runtime{format_str}, 42, 'x', "hello"sv);
      STATIC_CHECK(res == 14UL);
    }
    {
      constexpr emio::result<size_t> res = emio::formatted_size(precompiled_format_str, 42, 'x', "hello"sv);
      STATIC_CHECK(res == 14UL);
    }
    {
      constexpr size_t res = emio::formatted_size(precompiled_validated_format_str, 42, 'x', "hello"sv);
      STATIC_CHECK(res == 14UL);
    }
  }
}

TEST_CASE("runtime", "[format_string]") {
  // Test strategy:
  // * Construct an emio::runtime from different string types.
  // Expected: Everything works as expected.

  CHECK(emio::runtime<char>{}.view().empty());

  constexpr emio::runtime from_char_seq_at_cp{"12{3"};
  STATIC_CHECK(from_char_seq_at_cp.view() == "12{3");

  emio::runtime from_char_seq{"12{3"};
  CHECK(from_char_seq.view() == "12{3");

  emio::runtime from_string_view{"12{3"sv};
  CHECK(from_string_view.view() == "12{3");

  std::string s{"12{3"};
  emio::runtime from_string{s};
  CHECK(from_string.view() == "12{3");

  emio::format_string<int, char, std::string_view> str{emio::runtime{"{"}};
  CHECK(str.get() == emio::err::invalid_format);
  CHECK(str.as_valid() == emio::err::invalid_format);

  emio::result<emio::valid_format_string<int, char>> res = emio::valid_format_string<int, char>::from("{}");
  CHECK(res == emio::err::invalid_format);

  res = emio::valid_format_string<int, char>::from("{} {}");
  REQUIRE(res);
  CHECK(res->get() == "{} {}");
}
