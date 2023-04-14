#pragma GCC optimize("O3")

// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <string>

TEST_CASE("format double with emio vs fmt") {
  static constexpr std::array values = {
      0.0,
      1.0,
      12.0,
      123.0,
      1230.0,
      12300.0,
      123456789098765.0,
      1234567890987653.0,
      9.999999999999999e22,
      1.2,
      12.34,
      123.56,
      123456789098765.5,
      1234567890987653.9,
      0.99999,
      0.1,
      0.012,
      0.00123,
      0.00000123,
      std::numeric_limits<double>::min(),
      std::numeric_limits<double>::max(),
      std::numeric_limits<double>::lowest(),
      M_PI,
  };

  auto test = [](double value) {
    static constexpr std::array test_formats = {"", "g", "e", "f"};

    for (auto precision : {"", ".0", ".4"}) {
      for (auto alternate : {"", "#"}) {
        for (auto test_format : test_formats) {
          auto format_string = fmt::format("{{:{}{}{}}}", alternate, precision, test_format);
          for (double sign : {-1.0, 1.0}) {
            value *= sign;
            SECTION(fmt::format("emio vs fmt for {} format as {}", value, format_string)) {
              auto fmt_result = fmt::format(fmt::runtime(format_string), value);
              auto emio_result = emio::format(emio::runtime(format_string), value);
              REQUIRE(emio_result);
              CHECK(emio_result.value() == fmt_result);
            }
          }
        }
      }
    }
  };

  for (double value : values) {
    test(value);
  }
}

TEST_CASE("format double for shortest results in different rounding") {
  double d1 = 1234567890987653.25;
  double d2 = 1234567890987654.25;

  // Same result in Rust. Algorithm seems to always round up on 0.5.
  CHECK(emio::format("{}", d1) == "1234567890987653.3");
  CHECK(emio::format("{}", d2) == "1234567890987654.3");

  // Algorithm seems to always round down on 0.5.
  CHECK(fmt::format("{}", d1) == "1234567890987653.2");
  CHECK(fmt::format("{}", d2) == "1234567890987654.2");
}
