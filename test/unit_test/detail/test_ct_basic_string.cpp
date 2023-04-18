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

      ct_basic_string<char, 0> str;
      result &= str.size() == 0;
      result &= str.capacity() == 0;

      str.resize(5);
      result &= str.size() == 5;
      result &= str.capacity() == 5;
      *(str.data() + 2) = 'x';

      str.resize(10);
      result &= str.size() == 10;
      result &= str.capacity() == 10;
      *(str.data() + 8) = 'y';

      str.resize(15);
      result &= str.size() == 15;
      result &= str.capacity() >= 15;
      *(str.data() + 12) = 'z';
      std::string_view sv{str.data(), str.size()};
      result &= sv == "\0\0x\0\0\0\0\0y\0\0\0z\0\0"sv;

      str.resize(9);
      result &= str.size() == 9;
      result &= str.capacity() == 15;
      sv = std::string_view{str.data(), str.size()};
      result &= sv == "\0\0x\0\0\0\0\0y"sv;

      str.resize(15);
      result &= str.size() == 15;
      result &= str.capacity() == 15;
      sv = std::string_view{str.data(), str.size()};
      result &= sv == "\0\0x\0\0\0\0\0y\0\0\0\0\0\0"sv;

      return result;
    }();
    STATIC_CHECK(success);
  }
  SECTION("runtime") {
    constexpr size_t InternalStorageSize = 5;

    size_t prev_cap{};
    const auto get_capacity = [&](size_t resize) {
      if (resize <= InternalStorageSize) {
        if (resize < prev_cap) {
          return prev_cap;
        }
        prev_cap = InternalStorageSize;
        return InternalStorageSize;
      }
      if (resize < prev_cap) {
        return prev_cap;
      }
      prev_cap = resize;
      return resize;
    };

    ct_basic_string<char, InternalStorageSize> str;
    CHECK(str.size() == 0);
    CHECK(str.capacity() == get_capacity(0));

    str.resize(5);
    CHECK(str.size() == 5);
    CHECK(str.capacity() == get_capacity(0));
    *(str.data() + 2) = 'x';

    str.resize(10);
    CHECK(str.size() == 10);
    CHECK(str.capacity() == get_capacity(10));
    *(str.data() + 8) = 'y';

    str.resize(15);
    CHECK(str.size() == 15);
    CHECK(str.capacity() == get_capacity(15));
    *(str.data() + 12) = 'z';
    std::string_view sv{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0\0\0z\0\0"sv);

    str.resize(9);
    CHECK(str.size() == 9);
    CHECK(str.capacity() == get_capacity(9));
    sv = std::string_view{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y"sv);

    str.resize(15);
    CHECK(str.size() == 15);
    CHECK(str.capacity() == get_capacity(15));
    sv = std::string_view{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0\0\0\0\0\0"sv);
  }
}
