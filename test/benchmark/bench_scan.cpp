// Unit under test.
#include <fmt/format.h>

#include <emio/scan.hpp>

// Other includes.
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cinttypes>
#include <cmath>

TEST_CASE("scan simple integer") {
  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    int i;
    return emio::scan("1", "{}", i);
  };
  BENCHMARK("emio runtime") {
    int i;
    return emio::scan("1", emio::runtime("{}"), i);
  };
  BENCHMARK("snprintf") {
    int i;
    return sscanf("1", "%d", &i);
  };
}

TEST_CASE("scan complex integer") {
  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    int i;
    return emio::scan("0x1049Fae2", "{:#x}", i);
  };
  BENCHMARK("emio runtime") {
    int i;
    return emio::scan("0x1049Fae2", emio::runtime("{:#x}"), i);
  };
  BENCHMARK("snprintf") {
    unsigned int i;
    return sscanf("0x1049Fae2", "0x%x", &i);
  };
}
