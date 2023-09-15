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
  static constexpr std::string_view input("8978612134175239201");

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    int64_t i;
    return emio::scan(input, "{}", i);
  };
  BENCHMARK("emio runtime") {
    int64_t i;
    return emio::scan(input, emio::runtime("{}"), i);
  };
  BENCHMARK("snprintf") {
    int64_t i;
    return sscanf(input.data(), "%" PRIi64, &i);
  };
}

TEST_CASE("scan complex hex") {
  static constexpr std::string_view input("7C9A702A5186EC21");

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    int64_t i;
    return emio::scan(input, "{:x}", i);
  };
  BENCHMARK("emio runtime") {
    int64_t i;
    return emio::scan(input, emio::runtime("{:x}"), i);
  };
  BENCHMARK("snprintf") {
    int64_t i;
    return sscanf(input.data(), "%" PRIx64, &i);
  };
}
