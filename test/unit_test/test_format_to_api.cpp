// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("emio::format_to with output iterator", "[format_to]") {
  // Test strategy:
  // * Call emio::format_to with all supported output iterator types.
  // Expected: The return types, values and the format results are correct.

  SECTION("raw ptr") {
    SECTION("compile-time") {
      constexpr bool success = [] {
        std::array<char, 2> arr{};

        emio::result<char*> res = emio::format_to(arr.begin(), "{}", 42);
        return res == arr.end() && arr[0] == '4' && arr[1] == '2';
      }();
      STATIC_CHECK(success);
    }
    SECTION("runtime") {
      std::array<char, 2> arr{};

      emio::result<char*> res = emio::format_to(arr.begin(), "{}", 42);
      CHECK(res);
      CHECK(arr[0] == '4');
      CHECK(arr[1] == '2');
    }
  }

  SECTION("iterator") {
    std::string s;
    s.resize(2);

    emio::result<std::string::iterator> res = emio::format_to(s.begin(), "{}", 42);
    REQUIRE(res == s.end());
    CHECK(s == "42");
  }

  SECTION("back_insert_iterator") {
    std::string s;

    emio::result<std::back_insert_iterator<std::string>> res = emio::format_to(std::back_inserter(s), "{}", 42);
    REQUIRE(res);
    CHECK(s == "42");
  }
}

TEST_CASE("emio::format_to with emio::buffer", "[format_to]") {
  // Test strategy:
  // * Call emio::format_to with an emio::span_buffer.
  // Expected: The return type, value and the format result is correct.

  SECTION("compile-time") {
    SECTION("success") {
      constexpr bool success = [] {
        std::array<char, 2> arr{};
        emio::span_buffer buf{arr};

        emio::result<void> res = emio::format_to(buf, "{}", 42);
        return res && buf.view() == "42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("eof") {
      constexpr bool eof = [] {
        std::array<char, 2> arr{};
        emio::span_buffer buf{arr};

        emio::result<void> res = emio::format_to(buf, "{}", 420);
        return res == emio::err::eof;
      }();
      STATIC_CHECK(eof);
    }
  }
  SECTION("runtime") {
    std::array<char, 2> arr{};
    emio::span_buffer buf{arr};

    SECTION("success") {
      emio::result<void> res = emio::format_to(buf, "{}", 42);
      REQUIRE(res);
      CHECK(buf.view() == "42");
    }
    SECTION("eof") {
      emio::result<void> res = emio::format_to(buf, "{}", 420);
      REQUIRE(res == emio::err::eof);
    }
  }
}

TEST_CASE("emio::format_to with emio::writer", "[format_to]") {
  // Test strategy:
  // * Call emio::format_to with an emio::writer.
  // Expected: The return type, value and the format result is correct.

  std::array<char, 2> arr{};
  emio::span_buffer buf{arr};
  emio::writer<char> wrt{buf};

  SECTION("success") {
    emio::result<void> res = emio::format_to(wrt, "{}", 42);
    REQUIRE(res);
    CHECK(buf.view() == "42");
  }
  SECTION("eof") {
    emio::result<void> res = emio::format_to(wrt, "{}", 420);
    REQUIRE(res == emio::err::eof);
  }
}

TEST_CASE("emio::vformat_to with output iterator", "[format_to]") {
  // Test strategy:
  // * Call emio::vformat_to with all supported output iterator types.
  // Expected: The return types, values and the format results are correct.

  SECTION("raw ptr") {
    std::array<char, 2> arr{};

    emio::result<char*> res = emio::vformat_to(arr.begin(), emio::make_format_args("{}", 42));
    REQUIRE(res == arr.end());
    CHECK(arr[0] == '4');
    CHECK(arr[1] == '2');
  }

  SECTION("iterator") {
    std::string s;
    s.resize(2);

    emio::result<std::string::iterator> res = emio::vformat_to(s.begin(), emio::make_format_args("{}", 42));
    REQUIRE(res == s.end());
    CHECK(s == "42");
  }

  SECTION("back_insert_iterator") {
    std::string s;

    emio::result<std::back_insert_iterator<std::string>> res =
        emio::vformat_to(std::back_inserter(s), emio::make_format_args("{}", 42));
    REQUIRE(res);
    CHECK(s == "42");
  }
}

TEST_CASE("emio::vformat_to with emio::buffer", "[format_to]") {
  // Test strategy:
  // * Call emio::vformat_to with an emio::span_buffer.
  // Expected: The return type, value and the format result is correct.

  std::array<char, 2> arr{};
  emio::span_buffer buf{arr};

  SECTION("success") {
    emio::result<void> res = emio::vformat_to(buf, emio::make_format_args("{}", 42));
    REQUIRE(res);
    CHECK(buf.view() == "42");
  }
  SECTION("eof") {
    emio::result<void> res = emio::vformat_to(buf, emio::make_format_args("{}", 420));
    REQUIRE(res == emio::err::eof);
  }
}

TEST_CASE("emio::vformat_to with emio::writer", "[format_to]") {
  // Test strategy:
  // * Call emio::vformat_to with an emio::writer.
  // Expected: The return type, value and the format result is correct.

  std::array<char, 2> arr{};
  emio::span_buffer buf{arr};
  emio::writer<char> wrt{buf};

  SECTION("success") {
    emio::result<void> res = emio::vformat_to(wrt, emio::make_format_args("{}", 42));
    REQUIRE(res);
    CHECK(buf.view() == "42");
  }
  SECTION("eof") {
    emio::result<void> res = emio::vformat_to(wrt, emio::make_format_args("{}", 420));
    REQUIRE(res == emio::err::eof);
  }
}
