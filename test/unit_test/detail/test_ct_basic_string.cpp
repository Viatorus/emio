// Unit under test.
#include <emio/detail/basic_string.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("ct_basic_string") {
  // Test strategy:
  // * Construct, reserve, write into a ct_basic_string.
  // Expected: Memory is correctly managed.

  using emio::detail::ct_basic_string;

  SECTION("compile-time") {
    constexpr bool success = [] {
      bool result = true;

      ct_basic_string<char, 0> str;
      result &= str.size() == 0;
      result &= str.capacity() == 0;

      str.reserve(5);
      result &= str.size() == 5;
      result &= str.capacity() == 5;
      std::fill(str.data(), str.data() + str.size(), '\0');
      *(str.data() + 2) = 'x';

      str.reserve(10);
      result &= str.size() == 10;
      result &= str.capacity() == 10;
      std::fill(str.data() + 5, str.data() + str.size(), '\0');
      *(str.data() + 8) = 'y';

      str.reserve(15);
      result &= str.size() == 15;
      result &= str.capacity() >= 15;
      std::fill(str.data() + 10, str.data() + str.size(), '\0');
      *(str.data() + 12) = 'z';
      std::string_view sv{str.data(), str.size()};
      result &= sv == "\0\0x\0\0\0\0\0y\0\0\0z\0\0"sv;

      str.reserve(9);
      result &= str.size() == 9;
      result &= str.capacity() == 15;
      sv = std::string_view{str.data(), str.size()};
      result &= sv == "\0\0x\0\0\0\0\0y"sv;

      str.reserve(15);
      result &= str.size() == 15;
      result &= str.capacity() == 15;
      std::fill(str.data() + 10, str.data() + str.size(), '\0');
      sv = std::string_view{str.data(), str.size()};
      result &= sv == "\0\0x\0\0\0\0\0y\0\0\0\0\0\0"sv;

      return result;
    }();
    STATIC_CHECK(success);
  }
  SECTION("runtime") {
    constexpr size_t InternalStorageSize = 5;

    size_t prev_cap{};
    const auto get_capacity = [&](size_t reserve) {
      if (reserve <= InternalStorageSize) {
        if (reserve < prev_cap) {
          return prev_cap;
        }
        prev_cap = InternalStorageSize;
        return InternalStorageSize;
      }
      if (reserve < prev_cap) {
        return prev_cap;
      }
      prev_cap = reserve;
      return reserve;
    };

    ct_basic_string<char, InternalStorageSize> str;
    CHECK(str.size() == 0);
    CHECK(str.capacity() == get_capacity(0));

    str.reserve(5);
    CHECK(str.size() == 5);
    CHECK(str.capacity() == get_capacity(0));
    std::fill(str.data(), str.data() + str.size(), '\0');
    *(str.data() + 2) = 'x';

    str.reserve(10);
    CHECK(str.size() == 10);
    CHECK(str.capacity() == get_capacity(10));
    std::fill(str.data() + 5, str.data() + str.size(), '\0');
    *(str.data() + 8) = 'y';

    str.reserve(15);
    CHECK(str.size() == 15);
    CHECK(str.capacity() == get_capacity(15));
    std::fill(str.data() + 10, str.data() + str.size(), '\0');
    *(str.data() + 12) = 'z';
    std::string_view sv{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0\0\0z\0\0"sv);

    str.reserve(9);
    CHECK(str.size() == 9);
    CHECK(str.capacity() == get_capacity(9));
    sv = std::string_view{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y"sv);

    str.reserve(15);
    CHECK(str.size() == 15);
    CHECK(str.capacity() == get_capacity(15));
    sv = std::string_view{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0\0\0z\0\0"sv);
  }
}
