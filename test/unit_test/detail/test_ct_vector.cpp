// Unit under test.
#include <emio/detail/ct_vector.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("ct_vector") {
  // Test strategy:
  // * Construct, reserve, write into a ct_vector.
  // Expected: Memory is correctly managed.

  using emio::detail::ct_vector;

  SECTION("compile-time") {
    constexpr bool success = [] {
      bool result = true;

      ct_vector<char, 1> str;
      result &= str.size() == 0;
      result &= str.capacity() == 1;

      str.reserve(1);
      result &= str.size() == 1;
      result &= str.capacity() == 1;
      *str.data() = '1';

      str.reserve(5);
      result &= str.size() == 5;
      result &= str.capacity() == 5;
      std::fill(str.data() + 1, str.data() + str.size(), '\0');
      *(str.data() + 2) = 'x';

      str.reserve(4);
      result &= str.size() == 4;
      result &= str.capacity() == 5;

      str.reserve(5);
      result &= str.size() == 5;
      result &= str.capacity() == 5;

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
      result &= sv == "1\0x\0\0\0\0\0y\0\0\0z\0\0"sv;

      str.reserve(9);
      result &= str.size() == 9;
      result &= str.capacity() == 15;
      sv = std::string_view{str.data(), str.size()};
      result &= sv == "1\0x\0\0\0\0\0y"sv;

      str.reserve(15);
      result &= str.size() == 15;
      result &= str.capacity() == 15;
      std::fill(str.data() + 10, str.data() + str.size(), 'n');
      sv = std::string_view{str.data(), str.size()};
      result &= sv == "1\0x\0\0\0\0\0y\0nnnnn"sv;

      str.clear();
      result &= str.size() == 0;
      result &= str.capacity() == 15;

      str.reserve(1);
      result &= str.size() == 1;
      result &= str.capacity() == 15;

      str.reserve(15);
      result &= str.size() == 15;
      result &= str.capacity() == 15;
      sv = std::string_view{str.data(), str.size()};
      result &= sv == "1\0x\0\0\0\0\0y\0nnnnn"sv;

      // Copyable & moveable.
      {
        ct_vector<char, 1> str2{str};
        result &= str2.size() == 15;
        result &= str2.capacity() == 15;
        const std::string_view sv2{str2.data(), str.size()};
        result &= (sv == sv2);

        ct_vector<char, 1> str3;
        str3 = str2;
        result &= str3.size() == 15;
        result &= str3.capacity() == 15;
        const std::string_view sv3{str3.data(), str.size()};
        result &= (sv == sv3);

        ct_vector<char, 1> str4{std::move(str3)};
        result &= str4.size() == 15;
        result &= str4.capacity() == 15;
        const std::string_view sv4{str4.data(), str.size()};
        result &= (sv == sv4);

        ct_vector<char, 1> str5;
        str5 = std::move(str4);
        result &= str5.size() == 15;
        result &= str5.capacity() == 15;
        const std::string_view sv5{str5.data(), str.size()};
        result &= (sv == sv5);
      }

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

    ct_vector<char, InternalStorageSize> str;
    CHECK(str.size() == 0);
    CHECK(str.capacity() == get_capacity(0));

    str.reserve(5);
    CHECK(str.size() == 5);
    CHECK(str.capacity() == get_capacity(5));
    std::fill(str.data(), str.data() + str.size(), '\0');
    *(str.data() + 2) = 'x';

    str.reserve(4);
    CHECK(str.size() == 4);
    CHECK(str.capacity() == get_capacity(4));

    str.reserve(5);
    CHECK(str.size() == 5);
    CHECK(str.capacity() == get_capacity(5));

    str.reserve(10);
    CHECK(str.size() == 10);
    CHECK(str.capacity() == get_capacity(10));
    std::fill(str.data() + 5, str.data() + str.size(), '\0');
    *(str.data() + 8) = 'y';

    str.reserve(15);
    CHECK(str.size() == 15);
    CHECK(str.capacity() == get_capacity(15));
    std::fill(str.data() + 10, str.data() + str.size(), 'n');
    *(str.data() + 12) = 'z';
    std::string_view sv{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0nnznn"sv);

    str.reserve(9);
    CHECK(str.size() == 9);
    CHECK(str.capacity() == get_capacity(9));
    sv = std::string_view{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y"sv);

    str.reserve(15);
    CHECK(str.size() == 15);
    CHECK(str.capacity() == get_capacity(15));
    sv = std::string_view{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0nnznn"sv);

    str.clear();
    CHECK(str.size() == 0);
    CHECK(str.capacity() == 15);

    str.reserve(15);
    CHECK(str.size() == 15);
    CHECK(str.capacity() == get_capacity(15));
    sv = std::string_view{str.data(), str.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0nnznn"sv);

    // Copyable & moveable.
    {
      ct_vector<char, InternalStorageSize> str2{str};
      CHECK(str2.size() == 15);
      CHECK(str2.capacity() == get_capacity(15));
      const std::string_view sv2{str2.data(), str.size()};
      CHECK(sv == sv2);

      ct_vector<char, InternalStorageSize> str3;
      str3 = str2;
      CHECK(str3.size() == 15);
      CHECK(str3.capacity() == get_capacity(15));
      const std::string_view sv3{str3.data(), str.size()};
      CHECK(sv == sv3);

      ct_vector<char, InternalStorageSize> str4{std::move(str3)};
      CHECK(str4.size() == 15);
      CHECK(str4.capacity() == get_capacity(15));
      const std::string_view sv4{str4.data(), str.size()};
      CHECK(sv == sv4);

      ct_vector<char, InternalStorageSize> str5;
      str5 = std::move(str4);
      CHECK(str5.size() == 15);
      CHECK(str5.capacity() == get_capacity(15));
      const std::string_view sv5{str5.data(), str.size()};
      CHECK(sv == sv5);
    }
  }
}
