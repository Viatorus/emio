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
// - scan/format string
// - scan API docu
// - scan API md docu

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