// Unit under test.
#include <fmt/format.h>

#include <emio/format.hpp>

// Other includes.
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cinttypes>
#include <cmath>

static constexpr std::string_view long_text(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore "
    "magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
    "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est "
    "laborum.");


TEST_CASE("format nothing but long text") {
  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(long_text);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(long_text)).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(long_text);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(long_text));
  };
  BENCHMARK("snprintf") {
    std::string buf{};
    buf.resize(long_text.size() + 1);
    return snprintf(buf.data(), buf.size(), long_text.data());
  };
}

TEST_CASE("format string") {
  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format("{}", long_text);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime("{}"), long_text).value();
  };
  BENCHMARK("fmt") {
    return fmt::format("{}", long_text);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime("{}"), long_text);
  };
  BENCHMARK("snprintf") {
    std::string buf{};
    buf.resize(long_text.size() + 1);
    return snprintf(buf.data(), buf.size(), "%*s", static_cast<int>(long_text.size()), long_text.data());
  };
}

TEST_CASE("format small integer") {
  static constexpr std::string_view format_str(" {}");
  static constexpr int arg = 1;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    std::string buf{};
    buf.resize(42);
    return snprintf(buf.data(), buf.size(), " %d", 1);
  };
}

TEST_CASE("format big integer") {
  static constexpr std::string_view format_str(" {}");
  static constexpr int64_t arg = -8978612134175239201;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    std::string buf{};
    buf.resize(42);
    return snprintf(buf.data(), buf.size(), " %" PRIi64, arg);
  };
}

TEST_CASE("format big hex") {
  static constexpr std::string_view format_str("{:x}");
  static constexpr uint64_t arg = 8978612134175239201;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    std::string buf{};
    buf.resize(42);
    return snprintf(buf.data(), buf.size(), "%" PRIx64, arg);
  };
}

TEST_CASE("format complex format spec") {
  static constexpr std::string_view format_str("{0:x^+#20X}");
  static constexpr int64_t arg = 8978612134175239201;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
  // No snprintf equivalent.
}

TEST_CASE("format zero as double") {
  static constexpr std::string_view format_str("{}");
  static constexpr double arg = 0;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    std::string buf{};
    buf.resize(42);
    return snprintf(buf.data(), buf.size(), "%g", arg);
  };
}

TEST_CASE("format shortest double general") {
  static constexpr std::string_view format_str("{}");
  static constexpr double arg = M_PI;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf (not shortest but general)") {
    std::string buf{};
    buf.resize(42);
    return snprintf(buf.data(), buf.size(), "%g", arg);
  };
}

TEST_CASE("format double exponent") {
  static constexpr std::string_view format_str("{:e}");
  static constexpr double arg = M_PI;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    std::string buf{};
    buf.resize(42);
    return snprintf(buf.data(), buf.size(), "%e", arg);
  };
}

TEST_CASE("format double fixed") {
  static constexpr std::string_view format_str("{:f}");
  static constexpr double arg = M_PI;

  BENCHMARK("base") {
    return "1";
  };
  BENCHMARK("emio") {
    return emio::format(format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format(emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    std::string buf{};
    buf.resize(42);
    return snprintf(buf.data(), buf.size(), "%f", arg);
  };
}

TEST_CASE("format many arguments") {
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
    return emio::format(emio::runtime(format_str), ARGS).value();
  };
  BENCHMARK("fmt") {
    return fmt::format(format_str, ARGS);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format(fmt::runtime(format_str), ARGS);
  };
}
