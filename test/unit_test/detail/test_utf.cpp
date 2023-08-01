// Unit under test.
#include <emio/detail/utf.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

namespace {

[[nodiscard]] bool test_escape(std::string_view input, std::string_view expected) {
  const size_t escape_cnt = emio::detail::count_size_when_escaped(input);
  REQUIRE(escape_cnt == expected.size());

  std::string area(escape_cnt, '0');
  emio::detail::write_escaped_helper helper{input};
  const size_t written = helper.write_escaped(area);
  REQUIRE(written == escape_cnt);
  return area == expected;
}

}  // namespace

using namespace std::string_view_literals;

TEST_CASE("write_escaped") {
  SECTION("no escape") {
    constexpr std::string_view input = " !/09:@AZ[`az~";
    constexpr std::string_view expected = input;
    CHECK(test_escape(input, expected));
  }
  SECTION("backspace escaped") {
    constexpr std::string_view input = "\n\r\t\\'\"";
    constexpr std::string_view expected = "\\n\\r\\t\\\\\\'\\\"";
    CHECK(test_escape(input, expected));
  }
  SECTION("full escaped") {
    constexpr std::string_view input = "\x00\x19\x7f\xff"sv;
    constexpr std::string_view expected = "\\x00\\x19\\x7f\\xff";
    CHECK(test_escape(input, expected));
  }
  SECTION("all") {
    constexpr std::string_view input =
        "x\x05\r\x13\tt\x80"
        "0"sv;
    constexpr std::string_view expected = "x\\x05\\r\\x13\\tt\\x800";
    CHECK(test_escape(input, expected));
  }

  SECTION("escaping chunk by chunk") {
    SECTION("no escape") {
      emio::detail::write_escaped_helper helper{"a"};

      SECTION("zero space") {
        CHECK(helper.write_escaped({}) == 0);

        // Escape remaining.
        std::array<char, 1> array{};
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == 'a');
      }
      SECTION("1 space") {
        std::array<char, 1> array{};
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == 'a');
      }
      // Nothing left.
      std::array<char, 4> array{};
      CHECK(helper.write_escaped(array) == 0);
    }
    SECTION("backslash escaped") {
      emio::detail::write_escaped_helper helper{"\n"};

      SECTION("0 space") {
        CHECK(helper.write_escaped({}) == 0);

        // Escape remaining.
        std::array<char, 2> array{};
        CHECK(helper.write_escaped(array) == 2);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'n');
      }
      SECTION("1 space") {
        std::array<char, 1> array{};
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == '\\');
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == 'n');
      }
      SECTION("1 space with one zero space") {
        std::array<char, 1> array{};
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == '\\');
        CHECK(helper.write_escaped({}) == 0);
        CHECK(array[0] == '\\');
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == 'n');
      }
      SECTION("2 spaces") {
        std::array<char, 2> array{};
        CHECK(helper.write_escaped(array) == 2);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'n');
      }
      SECTION("3 spaces") {
        std::array<char, 3> array{};
        CHECK(helper.write_escaped(array) == 2);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'n');
        CHECK(array[2] == '\0');
      }
      // Nothing left.
      std::array<char, 4> array{};
      CHECK(helper.write_escaped(array) == 0);
    }
    SECTION("full escaped") {
      emio::detail::write_escaped_helper helper{"\xf5"};

      SECTION("0 space") {
        CHECK(helper.write_escaped({}) == 0);

        // Escape remaining.
        std::array<char, 4> array{};
        CHECK(helper.write_escaped(array) == 4);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'x');
        CHECK(array[2] == 'f');
        CHECK(array[3] == '5');
      }
      SECTION("1 space") {
        std::array<char, 1> array{};
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == '\\');
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == 'x');
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == 'f');
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == '5');
      }
      SECTION("2 spaces") {
        std::array<char, 2> array{};
        CHECK(helper.write_escaped(array) == 2);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'x');
        CHECK(helper.write_escaped(array) == 2);
        CHECK(array[0] == 'f');
        CHECK(array[1] == '5');
      }
      SECTION("2 spaces with one zero space") {
        std::array<char, 2> array{};
        CHECK(helper.write_escaped(array) == 2);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'x');
        CHECK(helper.write_escaped({}) == 0);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'x');
        CHECK(helper.write_escaped(array) == 2);
        CHECK(array[0] == 'f');
        CHECK(array[1] == '5');
      }
      SECTION("3 spaces") {
        std::array<char, 3> array{};
        CHECK(helper.write_escaped(array) == 3);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'x');
        CHECK(array[2] == 'f');
        CHECK(helper.write_escaped(array) == 1);
        CHECK(array[0] == '5');
      }
      SECTION("4 spaces") {
        std::array<char, 4> array{};
        CHECK(helper.write_escaped(array) == 4);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'x');
        CHECK(array[2] == 'f');
        CHECK(array[3] == '5');
      }
      SECTION("5 spaces") {
        std::array<char, 5> array{};
        CHECK(helper.write_escaped(array) == 4);
        CHECK(array[0] == '\\');
        CHECK(array[1] == 'x');
        CHECK(array[2] == 'f');
        CHECK(array[3] == '5');
        CHECK(array[4] == '\0');
      }
      // Nothing left.
      std::array<char, 4> array{};
      CHECK(helper.write_escaped(array) == 0);
    }
  }
}
