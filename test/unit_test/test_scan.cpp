// Unit under test.
#include <emio/scan.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

namespace {

template <typename... Args>
bool validate_scan_string(std::string_view str) {
  return emio::detail::scan::scan_trait::validate_string<Args...>(str);
}

}  // namespace

TEST_CASE("scan API", "[scan]") {
  SECTION("normal") {
    unsigned int a = 0;
    int b = 0;
    char c;
    REQUIRE(emio::scan("1,-2!", "{},{}{}", a, b, c));
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
  }
  SECTION("runtime string") {
    unsigned int a = 0;
    int b = 0;
    char c;

    REQUIRE(emio::scan("1,-2!", emio::runtime("{},{}{}"), a, b, c));
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
  }
  SECTION("compile-time") {
    constexpr bool success = [] {
      unsigned int a = 0;
      int b = 0;
      char c;

      emio::result<void> res = emio::scan("1,-2!", emio::runtime("{},{}{}"), a, b, c);
      return res && (a == 1) && (b == -2) && (c == '!');
    }();
    STATIC_CHECK(success);
  }
}

TEST_CASE("vscan API", "[scan]") {
  SECTION("normal") {
    unsigned int a = 0;
    int b = 0;
    char c;

    REQUIRE(emio::vscan("1,-2!", emio::make_scan_args("{},{}{}", a, b, c)));
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
  }
  SECTION("runtime string") {
    unsigned int a = 0;
    int b = 0;
    char c;

    REQUIRE(emio::vscan("1,-2!", emio::make_scan_args(emio::runtime("{},{}{}"), a, b, c)));
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
  }
}

TEST_CASE("scan_from API", "[scan]") {
  SECTION("normal") {
    unsigned int a = 0;
    int b = 0;
    char c;

    emio::reader rdr("1,-2!rest");
    REQUIRE(emio::scan_from(rdr, "{},{}{}", a, b, c));
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
    CHECK(rdr.read_remaining() == "rest");
  }
  SECTION("runtime string") {
    unsigned int a = 0;
    int b = 0;
    char c;

    emio::reader rdr("1,-2!rest");
    REQUIRE(emio::scan_from(rdr, emio::runtime("{},{}{}"), a, b, c));
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
    CHECK(rdr.read_remaining() == "rest");
  }
  SECTION("compile time") {
    constexpr bool success = [] {
      unsigned int a = 0;
      int b = 0;
      char c;

      emio::reader rdr("1,-2!rest");
      emio::result<void> res = emio::scan_from(rdr, emio::runtime("{},{}{}"), a, b, c);
      return res && (a == 1) && (b == -2) && (c == '!') && (rdr.read_remaining() == "rest");
    }();
    STATIC_CHECK(success);
  }
}

TEST_CASE("vscan_from API", "[scan]") {
  SECTION("normal") {
    unsigned int a = 0;
    int b = 0;
    char c;

    emio::reader rdr("1,-2!rest");
    REQUIRE(emio::vscan_from(rdr, emio::make_scan_args("{},{}{}", a, b, c)));
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
    CHECK(rdr.read_remaining() == "rest");
  }
  SECTION("runtime string") {
    unsigned int a = 0;
    int b = 0;
    char c;

    emio::reader rdr("1,-2!rest");
    REQUIRE(emio::vscan_from(rdr, emio::make_scan_args(emio::runtime("{},{}{}"), a, b, c)));
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
    CHECK(rdr.read_remaining() == "rest");
  }
}

TEST_CASE("incomplete scan", "[scan]") {
  int a = 0;
  int b = 0;

  SECTION("without reader") {
    CHECK(!emio::scan("1,-2rest", "{},{}", a, b));
  }
  SECTION("with reader") {
    emio::reader rdr("1,-2rest");
    CHECK(emio::scan_from(rdr, "{},{}", a, b));
    CHECK(rdr.read_remaining() == "rest");
  }
}

TEST_CASE("scan char", "[scan]") {
  char c;
  REQUIRE(emio::scan("o", "{}", c));
  CHECK(c == 'o');

  REQUIRE(emio::scan("k", "{:c}", c));
  CHECK(c == 'k');

  CHECK(validate_scan_string<char>("{:c}"));
  CHECK(!validate_scan_string<char>("{:d}"));
  CHECK(!validate_scan_string<char>("{:#}"));
}

TEST_CASE("detect base", "[scan]") {
  int val{};

  SECTION("binary") {
    REQUIRE(emio::scan("0b1011", "{:#}", val));
    CHECK(val == 0b1011);

    REQUIRE(emio::scan("0B101", "{:#}", val));
    CHECK(val == 0b101);

    REQUIRE(emio::scan("+0b1010", "{:#}", val));
    CHECK(val == +0b1010);

    REQUIRE(emio::scan("-0b111", "{:#}", val));
    CHECK(val == -0b111);
  }
  SECTION("octal") {
    REQUIRE(emio::scan("0175", "{:#}", val));
    CHECK(val == 0175);

    REQUIRE(emio::scan("+054", "{:#}", val));
    CHECK(val == +054);

    REQUIRE(emio::scan("-0023", "{:#}", val));
    CHECK(val == -023);
  }
  SECTION("decimal") {
    REQUIRE(emio::scan("-0", "{:#}", val));
    CHECK(val == 0);

    REQUIRE(emio::scan("0", "{:#}", val));
    CHECK(val == 0);

    REQUIRE(emio::scan("1", "{:#}", val));
    CHECK(val == 1);

    REQUIRE(emio::scan("+1", "{:#}", val));
    CHECK(val == +1);

    REQUIRE(emio::scan("-1", "{:#}", val));
    CHECK(val == -1);
  }
  SECTION("hex") {
    REQUIRE(emio::scan("0x58", "{:#}", val));
    CHECK(val == 0x58);

    REQUIRE(emio::scan("0XAe", "{:#}", val));
    CHECK(val == 0xAE);

    REQUIRE(emio::scan("+0x5", "{:#}", val));
    CHECK(val == +0x5);

    REQUIRE(emio::scan("-0x1", "{:#}", val));
    CHECK(val == -0x1);
  }
  SECTION("invalid") {
    CHECK(emio::scan("-", "{:#}", val) == emio::err::eof);
    CHECK(emio::scan("--1", "{:#}", val) == emio::err::invalid_data);
    CHECK(emio::scan("a", "{:#}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0xx", "{:#}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0x+1", "{:#}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0x-1", "{:#}", val) == emio::err::invalid_data);
  }
  SECTION("unsigned") {
    unsigned int uval{};
    CHECK(emio::scan("+5", "{:#}", uval));
    CHECK(uval == 5);

    CHECK(emio::scan("-1", "{:#}", uval) == emio::err::out_of_range);
    CHECK(emio::scan("-0x5", "{:#}", uval) == emio::err::out_of_range);
  }

  CHECK(validate_scan_string<int>("{:#x}"));
  CHECK(!validate_scan_string<int>("{:d#}"));
  CHECK(!validate_scan_string<int>("{:f}"));
  CHECK(!validate_scan_string<int>("{:.5}"));
}

TEST_CASE("scan_binary", "[scan]") {
  int val{};
  SECTION("no prefix") {
    REQUIRE(emio::scan("1011", "{:b}", val));
    CHECK(val == 0b1011);

    REQUIRE(emio::scan("+1001", "{:b}", val));
    CHECK(val == +0b1001);

    REQUIRE(emio::scan("-11", "{:b}", val));
    CHECK(val == -0b11);
  }
  SECTION("invalid number") {
    CHECK_FALSE(emio::scan("2", "{:b}", val));
  }
  SECTION("with prefix") {
    REQUIRE(emio::scan("0b10", "{:#b}", val));
    CHECK(val == 0b10);

    REQUIRE(emio::scan("0B110", "{:#b}", val));
    CHECK(val == 0b110);

    REQUIRE(emio::scan("+0b101", "{:#b}", val));
    CHECK(val == +0b101);

    REQUIRE(emio::scan("-0b101", "{:#b}", val));
    CHECK(val == -0b101);
  }
  SECTION("wrong prefix") {
    CHECK(emio::scan("0", "{:#b}", val) == emio::err::eof);
    CHECK(emio::scan("0o", "{:#b}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0x1", "{:#b}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0b+1", "{:#b}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0b-1", "{:#b}", val) == emio::err::invalid_data);
  }
}

TEST_CASE("scan_octal", "[scan]") {
  int val{};
  SECTION("no prefix") {
    REQUIRE(emio::scan("75", "{:o}", val));
    CHECK(val == 075);

    REQUIRE(emio::scan("+12", "{:o}", val));
    CHECK(val == +012);

    REQUIRE(emio::scan("-62", "{:o}", val));
    CHECK(val == -062);
  }
  SECTION("invalid number") {
    CHECK(emio::scan("8", "{:o}", val) == emio::err::invalid_data);
  }
  SECTION("with prefix") {
    REQUIRE(emio::scan("00", "{:#o}", val));
    CHECK(val == 00);

    REQUIRE(emio::scan("050", "{:#o}", val));
    CHECK(val == 050);

    REQUIRE(emio::scan("+050", "{:#o}", val));
    CHECK(val == +050);

    REQUIRE(emio::scan("-050", "{:#o}", val));
    CHECK(val == -050);
  }
  SECTION("wrong prefix") {
    CHECK(emio::scan("0", "{:#o}", val) == emio::err::eof);
    CHECK(emio::scan("0o", "{:#o}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0+0", "{:#o}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0-0", "{:#o}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0b3", "{:#o}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0x3", "{:#o}", val) == emio::err::invalid_data);
  }
}

TEST_CASE("scan_decimal", "[scan]") {
  int val{};
  SECTION("no prefix") {
    REQUIRE(emio::scan("49", "{}", val));
    CHECK(val == 49);

    REQUIRE(emio::scan("-87", "{}", val));
    CHECK(val == -87);

    REQUIRE(emio::scan("59849", "{:d}", val));
    CHECK(val == 59849);

    REQUIRE(emio::scan("+47891", "{:d}", val));
    CHECK(val == +47891);

    REQUIRE(emio::scan("-259", "{:d}", val));
    CHECK(val == -259);
  }
  SECTION("invalid number") {
    CHECK(emio::scan("a", "{:d}", val) == emio::err::invalid_data);
  }
  SECTION("with prefix") {
    REQUIRE(emio::scan("42", "{:#d}", val));
    CHECK(val == 42);

    REQUIRE(emio::scan("240", "{:#d}", val));
    CHECK(val == 240);

    REQUIRE(emio::scan("+42", "{:#d}", val));
    CHECK(val == +42);

    REQUIRE(emio::scan("-42", "{:#d}", val));
    CHECK(val == -42);
  }
  SECTION("edge cases") {
    val = -1;
    SECTION("test 1") {
      emio::reader rdr{"0x5"};
      REQUIRE(emio::scan_from(rdr, "{}", val));
      CHECK(val == 0);
      CHECK(rdr.read_remaining() == "x5");
    }

    SECTION("test 2") {
      emio::reader rdr{"0b3"};
      REQUIRE(emio::scan_from(rdr, "{:#d}", val));
      CHECK(val == 0);
      CHECK(rdr.read_remaining() == "b3");
    }

    SECTION("test 3") {
      REQUIRE(emio::scan("0", "{:#d}", val));
      CHECK(val == 0);
    }
  }
  SECTION("overflows") {
    SECTION("signed") {
      int8_t val8{};
      CHECK(emio::scan("127", "{}", val8));
      CHECK(val8 == 127);

      CHECK(emio::scan("128", "{}", val8) == emio::err::out_of_range);

      REQUIRE(emio::scan("-128", "{}", val8));
      CHECK(val8 == -128);

      CHECK(emio::scan("-129", "{}", val8) == emio::err::out_of_range);
    }
    SECTION("unsigned") {
      uint8_t uval8{};
      REQUIRE(emio::scan("255", "{}", uval8));
      CHECK(uval8 == 255);

      CHECK(emio::scan("256", "{}", uval8) == emio::err::out_of_range);
    }
  }
}

TEST_CASE("scan_hex", "[scan]") {
  int val{};
  SECTION("no prefix") {
    REQUIRE(emio::scan("Ad", "{:x}", val));
    CHECK(val == 0xAD);

    REQUIRE(emio::scan("+fe", "{:x}", val));
    CHECK(val == +0xfe);

    REQUIRE(emio::scan("-1d", "{:x}", val));
    CHECK(val == -0x1d);
  }
  SECTION("invalid number") {
    CHECK(emio::scan("g", "{:x}", val) == emio::err::invalid_data);
  }
  SECTION("with prefix") {
    REQUIRE(emio::scan("0xdf", "{:#x}", val));
    CHECK(val == 0xdf);

    REQUIRE(emio::scan("0X1e", "{:#x}", val));
    CHECK(val == 0x1e);

    REQUIRE(emio::scan("+0x1e", "{:#x}", val));
    CHECK(val == +0x1e);

    REQUIRE(emio::scan("-0x1e", "{:#x}", val));
    CHECK(val == -0x1e);
  }
  SECTION("wrong prefix") {
    CHECK(emio::scan("da", "{:#x}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0b3", "{:#x}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0o3", "{:#x}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0x+3", "{:#x}", val) == emio::err::invalid_data);
    CHECK(emio::scan("0x-3", "{:#x}", val) == emio::err::invalid_data);
  }
}
