// Unit under test.
#include <fmt/format.h>

#include <emio/format.hpp>

// Other includes.
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

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
  static constexpr int64_t arg = -8978612134175239201;

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

TEST_CASE("zero as double format") {
  static constexpr std::string_view format_str("{}");
  static constexpr double arg = 0;

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

TEST_CASE("shortest double general format") {
  static constexpr std::string_view format_str("{}");
  static constexpr double arg = M_PI;

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

TEST_CASE("double exponent format") {
  static constexpr std::string_view format_str("{:e}");
  static constexpr double arg = M_PI;

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

TEST_CASE("double fixed format") {
  static constexpr std::string_view format_str("{:f}");
  static constexpr double arg = M_PI;

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
  static constexpr std::string_view format_str("{} {} {} {} {} {} {} {} {} {} {}");
#define ARGS                                                                                                \
  true, static_cast<int8_t>(1), static_cast<uint8_t>(2), static_cast<int16_t>(3), static_cast<uint16_t>(4), \
      static_cast<int32_t>(5), static_cast<uint32_t>(6), "abc", 'x', nullptr, M_PI

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
