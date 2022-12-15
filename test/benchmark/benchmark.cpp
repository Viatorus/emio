// Unit under test.
#include <fmt/format.h>

#include <emio/format.hpp>

// Other includes.
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("simple integer format") {
  static constexpr std::string_view format_str(" {}");
  static constexpr int arg = 1;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime{format_str}, arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
}

TEST_CASE("complex integer format") {
  static constexpr std::string_view format_str("{0:x^+#10X}");
  static constexpr int arg = -89721;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime{format_str}, arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
}

TEST_CASE("many arguments") {
  static constexpr std::string_view format_str("{} {} {} {} {} {} {} {} {} {}");
#define ARGS                                                                                                \
  true, static_cast<int8_t>(1), static_cast<uint8_t>(2), static_cast<int16_t>(3), static_cast<uint16_t>(4), \
      static_cast<int32_t>(5), static_cast<uint32_t>(6), "abc", 'x', nullptr

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, ARGS);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime{format_str}, ARGS).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, ARGS);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), ARGS);
  };
}
