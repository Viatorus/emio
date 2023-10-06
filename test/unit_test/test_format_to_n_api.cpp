// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("emio::format_to_n with output iterator", "[format_to_n]") {
  // Test strategy:
  // * Call emio::format_to_n with all supported output iterator types.
  // Expected: The return types, values and the format results are correct.

  SECTION("raw ptr") {
    SECTION("compile-time") {
      constexpr bool success = [] {
        std::array<char, 2> arr{};

        emio::result<emio::format_to_n_result<char*>> res = emio::format_to_n(arr.begin(), 1, "{}", 421);
        return res && res->size == 3 && (res->out == arr.begin() + 1) && arr[0] == '4' && arr[1] == '\0';
      }();
      STATIC_CHECK(success);
    }
    SECTION("runtime") {
      std::array<char, 2> arr{};

      emio::result<emio::format_to_n_result<char*>> res = emio::format_to_n(arr.begin(), 1, "{}", 421);
      CHECK(res);
      CHECK(res->size == 3);
      CHECK(arr[0] == '4');
      CHECK(arr[1] == '\0');
    }
  }

  SECTION("iterator") {
    std::string s;
    s.resize(2);

    emio::result<emio::format_to_n_result<std::string::iterator>> res = emio::format_to_n(s.begin(), 1, "{}", 421);
    REQUIRE(res);
    CHECK(res->size == 3);
    CHECK(res->out == s.begin() + 1);
    CHECK(s == "4\0"sv);
  }

  SECTION("back_insert_iterator") {
    std::string s;

    emio::result<emio::format_to_n_result<std::back_insert_iterator<std::string>>> res =
        emio::format_to_n(std::back_inserter(s), 1, "{}", 421);
    REQUIRE(res);
    CHECK(res->size == 3);
    CHECK(s == "4");
  }
}

TEST_CASE("emio::vformat_to_n with output iterator", "[format_to_n]") {
  // Test strategy:
  // * Call emio::vformat_to with all supported output iterator types.
  // Expected: The return types, values and the format results are correct.

  SECTION("raw ptr") {
    std::array<char, 2> arr{};

    emio::result<emio::format_to_n_result<char*>> res =
        emio::vformat_to_n(arr.begin(), 1, emio::make_format_args("{}", 421));
    REQUIRE(res);
    CHECK(res->size == 3);
    CHECK(res->out == arr.begin() + 1);
    CHECK(arr[0] == '4');
    CHECK(arr[1] == '\0');
  }

  SECTION("iterator") {
    std::string s;
    s.resize(2);

    emio::result<emio::format_to_n_result<std::string::iterator>> res =
        emio::vformat_to_n(s.begin(), 1, emio::make_format_args("{}", 421));
    REQUIRE(res);
    CHECK(res->size == 3);
    CHECK(res->out == s.begin() + 1);
    CHECK(s == "4\0"sv);
  }

  SECTION("back_insert_iterator") {
    std::string s;

    emio::result<emio::format_to_n_result<std::back_insert_iterator<std::string>>> res =
        emio::vformat_to_n(std::back_inserter(s), 1, emio::make_format_args("{}", 421));
    REQUIRE(res);
    CHECK(res->size == 3);
    CHECK(s == "4");
  }
}

TEST_CASE("emio::vformat_to_n with buffer", "[format_to_n]") {
  // Test strategy:
  // * Write a longer message than the specified limit into a buffer.
  // Expected: The buffer only has a subset of the message and can later be used independently.
  emio::static_buffer<13> buf;

  emio::result<size_t> count = emio::vformat_to_n(buf, 10, emio::make_format_args("Hello {}!", 123456789));

  CHECK(count == 16U);
  CHECK(buf.view() == "Hello 1234");

  REQUIRE(emio::format_to(buf, "..."));
  CHECK(buf.view() == "Hello 1234...");
}

TEST_CASE("emio::format_to_n with buffer", "[format_to_n]") {
  // Test strategy:
  // * Write a longer message than the specified limit into a truncating buffer.
  // Expected: The primary buffer only has a subset of the message and can later be used independently.

  SECTION("runtime") {
    emio::static_buffer<13> buf;

    const emio::result<size_t> count = emio::format_to_n(buf, 10, "Hello {}!", 123456789);

    CHECK(count == 16U);
    CHECK(buf.view() == "Hello 1234");

    REQUIRE(emio::format_to(buf, "..."));
    CHECK(buf.view() == "Hello 1234...");
  }
  SECTION("compile-time") {
    constexpr bool success = [] {
      emio::static_buffer<10> buf{};
      const emio::result<size_t> count = emio::format_to_n(buf, 1, "{}", 42);
      return count == 2U && buf.view() == "4";
    }();
    STATIC_CHECK(success);
  }
}
