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
            std::tuple{"1", "0", "-1", "-10"},
            std::tuple{"0", "1", "2", "10"},
        },
        std::tuple{
            std::type_identity<int8_t>{},
            std::tuple{"-127", "-128", "-129", "-1280"},
            std::tuple{"126", "127", "128", "1270"},
        },
        std::tuple{
            std::type_identity<uint8_t>{},
            std::tuple{"1", "0", "-1", "-10"},
            std::tuple{"254", "255", "256", "2550"},
        },
        std::tuple{
            std::type_identity<int16_t>{},
            std::tuple{"-32767", "-32768", "-32769", "-327680"},
            std::tuple{"32766", "32767", "327678", "327670"},
        },
        std::tuple{
            std::type_identity<uint16_t>{},
            std::tuple{"1", "0", "-1", "-10"},
            std::tuple{"65534", "65535", "65536", "655350"},
        },
        std::tuple{
            std::type_identity<int32_t>{},
            std::tuple{"-2147483647", "-2147483648", "-2147483649", "-21474836480"},
            std::tuple{"2147483646", "2147483647", "2147483648", "21474836470"},
        },
        std::tuple{
            std::type_identity<uint32_t>{},
            std::tuple{"1", "0", "-1", "-10"},
            std::tuple{"4294967294", "4294967295", "4294967296", "42949672950"},
        },
        std::tuple{
            std::type_identity<int64_t>{},
            std::tuple{"-9223372036854775807", "-9223372036854775808", "-9223372036854775809", "-92233720368547758080"},
            std::tuple{"9223372036854775806", "9223372036854775807", "9223372036854775808", "-92233720368547758070"},
        },
        std::tuple{
            std::type_identity<uint64_t>{},
            std::tuple{"1", "0", "-1", "-10"},
            std::tuple{"18446744073709551614", "18446744073709551615", "18446744073709551616", "184467440737095516150"},
        },
    };

    const auto range_check = []<typename T>(std::type_identity<T> /*type*/, const auto& lower_input,
                                            const auto& upper_input) {
      CHECK(emio::reader{std::get<0>(lower_input)}.parse_int<T>() == std::numeric_limits<T>::min() + 1);
      CHECK(emio::reader{std::get<1>(lower_input)}.parse_int<T>() == std::numeric_limits<T>::min());
      CHECK(emio::reader{std::get<2>(lower_input)}.parse_int<T>() == emio::err::out_of_range);
      CHECK(emio::reader{std::get<3>(lower_input)}.parse_int<T>() == emio::err::out_of_range);

      CHECK(emio::reader{std::get<0>(upper_input)}.parse_int<T>() == std::numeric_limits<T>::max() - 1);
      CHECK(emio::reader{std::get<1>(upper_input)}.parse_int<T>() == std::numeric_limits<T>::max());
      CHECK(emio::reader{std::get<2>(upper_input)}.parse_int<T>() == emio::err::out_of_range);
      CHECK(emio::reader{std::get<3>(upper_input)}.parse_int<T>() == emio::err::out_of_range);
    };
    std::apply(
        [&](auto... inputs) {
          (std::apply(range_check, inputs), ...);
        },
        ranges);
  }

  SECTION("sign is correctly parsed") {
    SECTION("minus") {
      CHECK(emio::reader{"-"}.parse_int<int>() == emio::err::eof);
      CHECK(emio::reader{"-"}.parse_int<unsigned>() == emio::err::eof);
      CHECK(emio::reader{"- "}.parse_int<int>() == emio::err::invalid_data);
      CHECK(emio::reader{"- "}.parse_int<unsigned>() == emio::err::invalid_data);
      CHECK(emio::reader{"-8"}.parse_int<int>(8) == emio::err::invalid_data);
      CHECK(emio::reader{"-8"}.parse_int<unsigned>(8) == emio::err::invalid_data);
      CHECK(emio::reader{"-7"}.parse_int<int>(8) == -7);
      CHECK(emio::reader{"-7"}.parse_int<unsigned>(8) == emio::err::out_of_range);
    }
    SECTION("plus") {
      CHECK(emio::reader{"+"}.parse_int<int>() == emio::err::eof);
      CHECK(emio::reader{"+"}.parse_int<unsigned>() == emio::err::eof);
      CHECK(emio::reader{"+ "}.parse_int<int>() == emio::err::invalid_data);
      CHECK(emio::reader{"+ "}.parse_int<unsigned>() == emio::err::invalid_data);
      CHECK(emio::reader{"+8"}.parse_int<int>(8) == emio::err::invalid_data);
      CHECK(emio::reader{"+8"}.parse_int<unsigned>(8) == emio::err::invalid_data);
      CHECK(emio::reader{"+7"}.parse_int<int>(8) == 7);
      CHECK(emio::reader{"+7"}.parse_int<unsigned>(8) == 7U);
    }
    SECTION("both") {
      CHECK(emio::reader{"+-1"}.parse_int<int>() == emio::err::invalid_data);
      CHECK(emio::reader{"-+1"}.parse_int<unsigned>() == emio::err::invalid_data);
    }
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

TEST_CASE("reader::subreader", "[reader]") {
  SECTION("from empty reader") {
    emio::reader rdr;

    SECTION("defaulted") {
      emio::result<emio::reader> res = rdr.subreader(0);
      REQUIRE(res);
      CHECK(res->eof());

      res->unpop();
      CHECK(res->eof());

      res->pop();
      CHECK(res->eof());
    }
    SECTION("normal") {
      const size_t len = GENERATE(0U, 1U, 2U, emio::reader::npos);

      emio::result<emio::reader> res = rdr.subreader(0, len);
      REQUIRE(res);
      CHECK(res->eof());
    }
    SECTION("eof") {
      CHECK(rdr.subreader(1, 0) == emio::err::eof);
    }
  }
  SECTION("from non-empty reader") {
    constexpr std::string_view expected{"abc"};
    emio::reader rdr{expected};

    SECTION("defaulted") {
      emio::result<emio::reader> res = rdr.subreader(0);
      REQUIRE(res);
      CHECK(res->view_remaining() == expected);

      res->unpop();
      CHECK(res->view_remaining() == expected);

      res->pop();
      CHECK(res->view_remaining() == expected.substr(1));
    }
    SECTION("normal") {
      const size_t pos = GENERATE(0U, 1U, 2U, 3U);
      const size_t len = GENERATE(0U, 1U, 2U, 3U, emio::reader::npos);

      emio::result<emio::reader> res = rdr.subreader(pos, len);
      REQUIRE(res);
      CHECK(res->view_remaining() == expected.substr(pos, len));
    }
    SECTION("eof") {
      CHECK(rdr.subreader(expected.size() + 1) == emio::err::eof);
    }
  }
  SECTION("from non-empty reader after some reading") {
    emio::reader rdr{"123abc"};
    REQUIRE(rdr.read_n_chars(3));
    constexpr std::string_view expected{"abc"};

    SECTION("defaulted") {
      emio::result<emio::reader> res = rdr.subreader(0);
      REQUIRE(res);
      CHECK(res->view_remaining() == expected);

      res->unpop();
      CHECK(res->view_remaining() == expected);

      res->pop();
      CHECK(res->view_remaining() == expected.substr(1));
    }
    SECTION("normal") {
      const size_t pos = GENERATE(0U, 1U, 2U, 3U);
      const size_t len = GENERATE(0U, 1U, 2U, 3U, emio::reader::npos);

      emio::result<emio::reader> res = rdr.subreader(pos, len);
      REQUIRE(res);
      CHECK(res->view_remaining() == expected.substr(pos, len));
    }
    SECTION("eof") {
      CHECK(rdr.subreader(expected.size() + 1) == emio::err::eof);
    }
  }
}
