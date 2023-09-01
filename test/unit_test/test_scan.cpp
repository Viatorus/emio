// Unit under test.
#include <emio/scan.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

// TODO:
// - support hex, octal, binary, + prefix
// - support string
// - unit tests scan, vscan, vscan_from
// - benchmark test
// - size test
// - scan/format string naming
// - scan API docu
// - scan API user docu

TEST_CASE("scan", "[scan]") {
  int a = 0;
  int b = 0;
  char c;
  emio::result<void> r = emio::scan("1,-2!", "{},{}{}", a, b, c);
  REQUIRE(r);
  CHECK(a == 1);
  CHECK(b == -2);
  CHECK(c == '!');

  emio::reader rdr("1,-2!REST");
  r = emio::scan_from(rdr, emio::runtime("{},{}{}"), a, b, c);
  REQUIRE(r);
  CHECK(a == 1);
  CHECK(b == -2);
  CHECK(c == '!');
  CHECK(rdr.read_remaining() == "REST");
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

    CHECK(emio::scan("-1", "{:#}", uval) == emio::err::invalid_data);
    CHECK(emio::scan("-0x5", "{:#}", uval) == emio::err::invalid_data);
  }
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
  SECTION("wrong prefix (not applicable)") {
    CHECK(emio::scan("0x", "{:#d}", val));
    CHECK(val == 0);
    CHECK(emio::scan("0", "{:#d}", val));
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
