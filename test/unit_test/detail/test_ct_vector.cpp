// Unit under test.
#include <emio/detail/ct_vector.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

using namespace std::string_view_literals;

namespace {

// Copyable & moveable.
template <typename T>
void check_equality_of_vector(T& expected, T& other) {
  const auto compare = [&]() {
    CHECK(expected.size() == other.size());
    CHECK(expected.capacity() >= other.capacity());
    const std::string_view sv{expected.data(), expected.size()};
    const std::string_view sv2{other.data(), other.size()};
    CHECK(sv == sv2);
  };

  compare();

  expected.reserve(expected.size() + 2);
  other.reserve(other.size() + 2);
  std::fill(expected.data() + expected.size() - 2, expected.data() + expected.size(), '1');
  std::fill(other.data() + other.size() - 2, other.data() + other.size(), '1');

  compare();

  expected.reserve(expected.size() + 123);
  other.reserve(other.size() + 123);
  std::fill(expected.data() + expected.size() - 123, expected.data() + expected.size(), '2');
  std::fill(other.data() + other.size() - 123, other.data() + other.size(), '2');

  compare();
}

template <typename T>
void check_gang_of_5(T& buf) {
  SECTION("copy-construct") {
    T buf2{buf};
    check_equality_of_vector(buf, buf2);
  }
  SECTION("copy-assign") {
    T buf2;
    buf2 = buf;
    check_equality_of_vector(buf, buf2);
  }
  SECTION("move-construct") {
    T buf_tmp{buf};
    T buf2{std::move(buf_tmp)};
    check_equality_of_vector(buf, buf2);
  }
  SECTION("move-assign") {
    T buf_tmp{buf};
    T buf2;
    buf2 = std::move(buf_tmp);
    check_equality_of_vector(buf, buf2);
  }
  SECTION("wild") {
    T buf2{buf};
    T buf3{std::move(buf2)};
    T buf4;
    buf4 = buf3;
    T buf5;
    buf5 = std::move(buf4);
    T buf6{buf};
    buf6 = buf5;

    SECTION("1") {
      check_equality_of_vector(buf, buf5);
    }
    SECTION("2") {
      check_equality_of_vector(buf6, buf5);
    }
  }
  SECTION("self-assignment copy") {
    T buf2{buf};
    T& buf_tmp = buf2;  // Prevent compiler warn about self assignment.
    buf2 = buf_tmp;
    check_equality_of_vector(buf, buf2);
  }
  SECTION("self-assignment move") {
    T buf2{buf};
    T& buf_tmp = buf2;  // Prevent compiler warn about self assignment.
    buf2 = std::move(buf_tmp);
    check_equality_of_vector(buf, buf2);
  }
}

}  // namespace

TEST_CASE("ct_vector") {
  // Test strategy:
  // * Construct, reserve, write into a ct_vector.
  // Expected: Memory is correctly managed.

  using emio::detail::ct_vector;

  SECTION("compile-time") {
    constexpr bool success = [] {
      bool result = true;

      ct_vector<char, 1> vec;
      result &= vec.size() == 0;
      result &= vec.capacity() == 1;

      vec.reserve(1);
      result &= vec.size() == 1;
      result &= vec.capacity() == 1;
      *vec.data() = '1';

      vec.reserve(5);
      result &= vec.size() == 5;
      result &= vec.capacity() == 5;
      std::fill(vec.data() + 1, vec.data() + vec.size(), '\0');
      *(vec.data() + 2) = 'x';

      vec.reserve(4);
      result &= vec.size() == 4;
      result &= vec.capacity() == 5;

      vec.reserve(5);
      result &= vec.size() == 5;
      result &= vec.capacity() == 5;

      vec.reserve(10);
      result &= vec.size() == 10;
      result &= vec.capacity() == 10;
      std::fill(vec.data() + 5, vec.data() + vec.size(), '\0');
      *(vec.data() + 8) = 'y';

      vec.reserve(15);
      result &= vec.size() == 15;
      result &= vec.capacity() >= 15;
      std::fill(vec.data() + 10, vec.data() + vec.size(), '\0');
      *(vec.data() + 12) = 'z';
      std::string_view sv{vec.data(), vec.size()};
      result &= sv == "1\0x\0\0\0\0\0y\0\0\0z\0\0"sv;

      vec.reserve(9);
      result &= vec.size() == 9;
      result &= vec.capacity() == 15;
      sv = std::string_view{vec.data(), vec.size()};
      result &= sv == "1\0x\0\0\0\0\0y"sv;

      vec.reserve(15);
      result &= vec.size() == 15;
      result &= vec.capacity() == 15;
      std::fill(vec.data() + 10, vec.data() + vec.size(), 'n');
      sv = std::string_view{vec.data(), vec.size()};
      result &= sv == "1\0x\0\0\0\0\0y\0nnnnn"sv;

      vec.clear();
      result &= vec.size() == 0;
      result &= vec.capacity() == 15;

      vec.reserve(1);
      result &= vec.size() == 1;
      result &= vec.capacity() == 15;

      vec.reserve(15);
      result &= vec.size() == 15;
      result &= vec.capacity() == 15;
      sv = std::string_view{vec.data(), vec.size()};
      result &= sv == "1\0x\0\0\0\0\0y\0nnnnn"sv;

      // Copyable & moveable.
      {
        ct_vector<char, 1> str2{vec};
        result &= str2.size() == 15;
        result &= str2.capacity() == 15;
        const std::string_view sv2{str2.data(), vec.size()};
        result &= (sv == sv2);

        ct_vector<char, 1> str3;
        str3 = str2;
        result &= str3.size() == 15;
        result &= str3.capacity() == 15;
        const std::string_view sv3{str3.data(), vec.size()};
        result &= (sv == sv3);

        ct_vector<char, 1> str4{std::move(str3)};
        result &= str4.size() == 15;
        result &= str4.capacity() == 15;
        const std::string_view sv4{str4.data(), vec.size()};
        result &= (sv == sv4);

        ct_vector<char, 1> str5;
        str5 = std::move(str4);
        result &= str5.size() == 15;
        result &= str5.capacity() == 15;
        const std::string_view sv5{str5.data(), vec.size()};
        result &= (sv == sv5);
      }

      return result;
    }();
    STATIC_CHECK(success);
  }

  SECTION("runtime") {
    constexpr size_t InternalStorageSize = 5;

    const int checkpoint = GENERATE(range(0, 7));
    INFO("Checkpoint: " << checkpoint);

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

    ct_vector<char, InternalStorageSize> vec;
    CHECK(vec.size() == 0);
    CHECK(vec.capacity() == get_capacity(0));

    if (checkpoint == 0) {
      check_gang_of_5(vec);
      return;
    }

    vec.reserve(5);
    CHECK(vec.size() == 5);
    CHECK(vec.capacity() == get_capacity(5));
    std::fill(vec.data(), vec.data() + vec.size(), '\0');
    *(vec.data() + 2) = 'x';

    if (checkpoint == 1) {
      check_gang_of_5(vec);
      return;
    }

    vec.reserve(4);
    CHECK(vec.size() == 4);
    CHECK(vec.capacity() == get_capacity(4));

    vec.reserve(5);
    CHECK(vec.size() == 5);
    CHECK(vec.capacity() == get_capacity(5));

    vec.reserve(10);
    CHECK(vec.size() == 10);
    CHECK(vec.capacity() == get_capacity(10));
    std::fill(vec.data() + 5, vec.data() + vec.size(), '\0');
    *(vec.data() + 8) = 'y';

    if (checkpoint == 2) {
      check_gang_of_5(vec);
      return;
    }

    vec.reserve(15);
    CHECK(vec.size() == 15);
    CHECK(vec.capacity() == get_capacity(15));
    std::fill(vec.data() + 10, vec.data() + vec.size(), 'n');
    *(vec.data() + 12) = 'z';
    std::string_view sv{vec.data(), vec.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0nnznn"sv);

    if (checkpoint == 3) {
      check_gang_of_5(vec);
      return;
    }

    vec.reserve(9);
    CHECK(vec.size() == 9);
    CHECK(vec.capacity() == get_capacity(9));
    sv = std::string_view{vec.data(), vec.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y"sv);

    if (checkpoint == 4) {
      check_gang_of_5(vec);
      return;
    }

    vec.reserve(15);
    CHECK(vec.size() == 15);
    CHECK(vec.capacity() == get_capacity(15));
    sv = std::string_view{vec.data(), vec.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0nnznn"sv);

    if (checkpoint == 5) {
      check_gang_of_5(vec);
      return;
    }

    vec.clear();
    CHECK(vec.size() == 0);
    CHECK(vec.capacity() == 15);

    vec.reserve(15);
    CHECK(vec.size() == 15);
    CHECK(vec.capacity() == get_capacity(15));
    sv = std::string_view{vec.data(), vec.size()};
    CHECK(sv == "\0\0x\0\0\0\0\0y\0nnznn"sv);

    check_gang_of_5(vec);
  }
}
