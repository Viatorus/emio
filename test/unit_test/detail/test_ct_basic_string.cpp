// Unit under test.
#include <emio/detail/basic_string.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("ct_basic_string") {
  // Test strategy:
  // * Construct, resize, write into a ct_basic_string.
  // Expected: Memory is correctly managed.

  using emio::detail::ct_basic_string;

  SECTION("compile-time") {
    constexpr bool success = [] {
      bool result = true;

      ct_basic_string<char> str;
      str.resize(5);
      *(str.data() + 2) = 'x';

      str.resize(10);
      *(str.data() + 8) = 'y';

      str.resize(15);
      *(str.data() + 12) = 'z';
      std::string_view sv{str.data(), str.size()};
      result &= sv == "\0\0x\0\0\0\0\0y\0\0\0z\0\0"sv;

      str.resize(9);
      sv = std::string_view{str.data(), str.size()};
      result &= sv == "\0\0x\0\0\0\0\0y"sv;

      return result;
    }();
    STATIC_CHECK(success);
  }
  SECTION("runtime") {
    ct_basic_string<char> str;
    str.resize(5);
    *(str.data() + 2) = 'x';

    str.resize(10);
    *(str.data() + 8) = 'y';

    str.resize(15);
    *(str.data() + 12) = 'z';
    std::string_view sv{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0\0\0z\0\0"sv);

    str.resize(9);
    sv = std::string_view{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y"sv);
  }
}
