// Unit under test.
#include <emio/reader.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std::string_view_literals;

TEST_CASE("reader", "[reader]") {
  // Test strategy:
  // * Call all trivial read methods.
  // Expected: The methods work as expected.

  constexpr std::string_view initial_str{"abc ghi def"};

  emio::reader reader{initial_str};

  CHECK(reader.pos() == 0);
  CHECK(reader.cnt_remaining() == 11);
  CHECK(reader.view_remaining() == initial_str);
  reader.unpop();
  CHECK(reader.view_remaining() == initial_str);

  CHECK(reader.peek() == 'a');
  reader.pop();
  CHECK(reader.peek() == 'b');
  reader.pop(3);
  CHECK(reader.pos() == 4);

  CHECK(reader.cnt_remaining() == 7);
  CHECK(reader.view_remaining() == "ghi def");

  CHECK(reader.read_if_match_char('h') == emio::err::invalid_data);
  CHECK(reader.read_if_match_char('g') == 'g');
  CHECK(reader.read_if_match_str("hi!") == emio::err::invalid_data);
  CHECK(reader.read_if_match_str("hi ") == "hi ");

  CHECK(reader.cnt_remaining() == 3);
  CHECK(reader.view_remaining() == "def");

  CHECK(reader.read_char() == 'd');
  CHECK(reader.read_n_chars(3) == emio::err::eof);
  reader.unpop();
  CHECK(reader.read_n_chars(3) == "def");
  CHECK(reader.peek() == emio::err::eof);
  CHECK(reader.read_char() == emio::err::eof);
  CHECK(reader.cnt_remaining() == 0);
  reader.unpop();

  CHECK(reader.cnt_remaining() == 1);
  CHECK_FALSE(reader.eof());
  CHECK(reader.peek() == 'f');
  CHECK(reader.read_char() == 'f');
  CHECK(reader.eof());
  CHECK(reader.cnt_remaining() == 0);
  CHECK(reader.read_remaining().empty());
  reader.unpop();

  CHECK(reader.cnt_remaining() == 1);
  CHECK(reader.read_remaining() == "f");

  CHECK(reader.read_if_match_char('e') == emio::err::eof);
  CHECK(reader.read_if_match_str("end") == emio::err::eof);

  reader.unpop(500);
  CHECK(reader.view_remaining() == initial_str);
}

TEST_CASE("reader::constructor", "[reader]") {
  // Test strategy:
  // * Construct the reader from different types of strings.
  // Expected: The deduction guide works and all behave the same.

  emio::reader s1{"abc"};
  std::string str{"abc"};
  emio::reader s2{str};
  emio::reader s3{"abc"sv};

  STATIC_CHECK(std::is_same_v<decltype(s1), decltype(s2)>);
  STATIC_CHECK(std::is_same_v<decltype(s2), decltype(s3)>);
  STATIC_CHECK(std::is_same_v<decltype(s3), emio::reader>);

  CHECK(s1.read_remaining() == "abc");
  CHECK(s2.read_remaining() == "abc");
  CHECK(s3.read_remaining() == "abc");
}

TEST_CASE("reader::parse_int", "[reader]") {
  // Test strategy:
  // * Call parse_int with different input, integer types and bases.
  // Expected: The method works as expected.

  SECTION("min/max cases") {
    constexpr std::tuple ranges{
        std::tuple{
            std::type_identity<bool>{},
            std::to_array({"1", "0", "-1"}),
            std::to_array({"0", "1", "2"}),
        },
        std::tuple{
            std::type_identity<int8_t>{},
            std::to_array({"-127", "-128", "-129"}),
            std::to_array({"126", "127", "128"}),
        },
        std::tuple{
            std::type_identity<uint8_t>{},
            std::to_array({"1", "0", "-1"}),
            std::to_array({"254", "255", "256"}),
        },
        std::tuple{
            std::type_identity<int16_t>{},
            std::to_array({"-32767", "-32768", "-32769"}),
            std::to_array({"32766", "32767", "327678"}),
        },
        std::tuple{
            std::type_identity<uint16_t>{},
            std::to_array({"1", "0", "-1"}),
            std::to_array({"65534", "65535", "65536"}),
        },
        std::tuple{
            std::type_identity<int32_t>{},
            std::to_array({"-2147483647", "-2147483648", "-2147483649"}),
            std::to_array({"2147483646", "2147483647", "2147483648"}),
        },
        std::tuple{
            std::type_identity<uint32_t>{},
            std::to_array({"1", "0", "-1"}),
            std::to_array({"4294967294", "4294967295", "4294967296"}),
        },
        std::tuple{
            std::type_identity<int64_t>{},
            std::to_array({"-9223372036854775807", "-9223372036854775808", "-9223372036854775809"}),
            std::to_array({"9223372036854775806", "9223372036854775807", "9223372036854775808"}),
        },
        std::tuple{
            std::type_identity<uint64_t>{},
            std::to_array({"1", "0", "-1"}),
            std::to_array({"18446744073709551614", "18446744073709551615", "18446744073709551616"}),
        },
    };

    const auto range_check = []<typename T>(std::type_identity<T> /*type*/, const auto& lower_input,
                                            const auto& upper_input) {
      CHECK(emio::reader{lower_input.at(0)}.parse_int<T>() == std::numeric_limits<T>::min() + 1);
      CHECK(emio::reader{lower_input.at(1)}.parse_int<T>() == std::numeric_limits<T>::min());
      CHECK(emio::reader{lower_input.at(2)}.parse_int<T>() == emio::err::out_of_range);

      CHECK(emio::reader{upper_input.at(0)}.parse_int<T>() == std::numeric_limits<T>::max() - 1);
      CHECK(emio::reader{upper_input.at(1)}.parse_int<T>() == std::numeric_limits<T>::max());
      CHECK(emio::reader{upper_input.at(2)}.parse_int<T>() == emio::err::out_of_range);
    };
    std::apply(
        [&](auto... inputs) {
          (std::apply(range_check, inputs), ...);
        },
        ranges);
  }

  SECTION("just a -") {
    CHECK(emio::reader{"-"}.parse_int<int>() == emio::err::eof);
    CHECK(emio::reader{"-"}.parse_int<unsigned>() == emio::err::out_of_range);
  }

  SECTION("a failed parse_int keeps previous read position") {
    emio::reader reader{"abc -348648"};
    CHECK(reader.read_n_chars(4) == "abc ");
    CHECK(reader.parse_int<uint8_t>() == emio::err::out_of_range);
    CHECK(reader.view_remaining() == "-348648");
    CHECK(reader.parse_int<int16_t>() == emio::err::out_of_range);
    CHECK(reader.view_remaining() == "-348648");
    CHECK(reader.parse_int<int32_t>() == -348648);
  }

  SECTION("base") {
    SECTION("positive") {
      auto str = "0159fE"sv;
      CHECK(emio::reader{str}.parse_int<int32_t>(10) == 159);
      CHECK(emio::reader{str}.parse_int<int32_t>(2) == 1);
      CHECK(emio::reader{str}.parse_int<int32_t>(8) == 13);
      CHECK(emio::reader{str}.parse_int<int32_t>(16) == 88574);
    }
    SECTION("negative") {
      auto str = "-1078aB"sv;
      CHECK(emio::reader{str}.parse_int<int32_t>(10) == -1078);
      CHECK(emio::reader{str}.parse_int<int32_t>(2) == -2);
      CHECK(emio::reader{str}.parse_int<int32_t>(8) == -0107);
      CHECK(emio::reader{str}.parse_int<int32_t>(16) == -1079467);
    }
    SECTION("invalid") {
      CHECK(emio::reader{"1"}.parse_int<int32_t>(1) == emio::err::invalid_argument);
      CHECK(emio::reader{"1"}.parse_int<int32_t>(37) == emio::err::invalid_argument);
    }
  }
}

TEST_CASE("reader::read_until", "[reader]") {
  SECTION("read_until_char") {
    emio::reader reader{"a12bcd"};

    auto res = reader.read_until_char('b');
    CHECK(res == "a12");
  }
  SECTION("read_until_str") {
    emio::reader reader{"abcabd123"};

    auto res = reader.read_until_str("abd");
    CHECK(res == "abc");
  }
  SECTION("read_until_any_of") {
    emio::reader reader{"a12bcd"};

    auto res = reader.read_until_any_of("cb");
    CHECK(res == "a12");
  }
  SECTION("read_until_none_of") {
    emio::reader reader{"a12bcd"};

    auto res = reader.read_until_none_of("12a");
    CHECK(res == "a12");
  }
  SECTION("read_until") {
    emio::reader reader{"a12bcd"};

    auto res = reader.read_until([](char c) {
      return c == 'b';
    });
    CHECK(res == "a12");
  }
}

TEST_CASE("reader::read_until_options", "[reader]") {
  // Test strategy:
  // * Call one read_until function with all possible combinations of options.
  // Expected: The options works as expected.

  SECTION("with a filled reader") {
    emio::reader reader{"a12bcd"};

    const bool should_succeed = GENERATE(true, false);
    const bool include_delimiter = GENERATE(true, false);
    const bool keep_delimiter = GENERATE(true, false);
    const bool ignore_eof = GENERATE(true, false);
    const char delimiter = should_succeed ? 'c' : 'x';

    const emio::reader::read_until_options options{
        .include_delimiter = include_delimiter,
        .keep_delimiter = keep_delimiter,
        .ignore_eof = ignore_eof,
    };
    emio::result<std::string_view> res = reader.read_until_char(delimiter, options);

    if (should_succeed) {
      if (include_delimiter) {
        CHECK(res == "a12bc");
      } else {
        CHECK(res == "a12b");
      }
      if (keep_delimiter) {
        CHECK(reader.read_remaining() == "cd");
      } else {
        CHECK(reader.read_remaining() == "d");
      }
    } else {
      if (ignore_eof) {
        CHECK(res == emio::err::invalid_data);
        CHECK(reader.read_remaining() == "a12bcd");
      } else {
        CHECK(res == "a12bcd");
        CHECK(reader.read_remaining().empty());
      }
    }
  }

  SECTION("with an empty reader") {
    emio::reader reader{""};

    const bool ignore_eof = GENERATE(true, false);
    const emio::reader::read_until_options options{
        .ignore_eof = ignore_eof,
    };

    emio::result<std::string_view> res = reader.read_until_char('x', options);
    CHECK(res == emio::err::eof);
  }

  SECTION("default option works with delimiter and EOF") {
    emio::reader reader{"0;1;2;3;4"};
    int i = 0;
    while (auto result = reader.read_until_char(';')) {
      // Should return the number without ;.
      // Parse integer ascending.
      emio::reader int_parser{result.value()};
      CHECK(int_parser.parse_int<int>() == i++);
      CHECK(int_parser.eof());
    }
    CHECK(i == 5);
  }
}
