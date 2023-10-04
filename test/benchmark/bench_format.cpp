// Unit under test.
#include <fmt/format.h>

#include <emio/format.hpp>

// Other includes.
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cinttypes>
#include <cmath>

static constexpr std::string_view long_text{
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore "
    "magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
    "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est "
    "laborum."};

TEST_CASE("format default") {
  static constexpr std::string_view format_str{"Hello world! Let's count together until {}!"};
  static constexpr int arg = 42;

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);
    return emio_str == fmt_str;
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
}

TEST_CASE("format nothing but long text") {
  static constexpr std::string_view format_str{long_text};

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str);
    const std::string fmt_str = fmt::format(format_str);
    REQUIRE(emio_str == fmt_str);

    REQUIRE(snprintf(buf.data(), buf.size(), format_str.data()) == static_cast<int>(emio_str.size()));
    REQUIRE(emio_str == buf.data());
    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str)).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str));
  };
  BENCHMARK("snprintf") {
    return snprintf(buf.data(), buf.size(), format_str.data());
  };
}

TEST_CASE("format string") {
  static constexpr std::string_view format_str{" {}"};
  static constexpr std::string_view arg = long_text;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);

    REQUIRE(snprintf(buf.data(), buf.size(), " %*s", static_cast<int>(arg.size()), arg.data()) ==
            static_cast<int>(emio_str.size()));
    REQUIRE(emio_str == buf.data());
    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg).value();
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    return snprintf(buf.data(), buf.size(), " %*s", static_cast<int>(arg.size()), arg.data());
  };
}

TEST_CASE("format small integer") {
  static constexpr std::string_view format_str{" {}"};
  static constexpr int arg = 1;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);

    REQUIRE(snprintf(buf.data(), buf.size(), " %d", arg) == static_cast<int>(emio_str.size()));
    REQUIRE(emio_str == buf.data());
    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    return snprintf(buf.data(), buf.size(), " %d", arg);
  };
}

TEST_CASE("format big integer") {
  static constexpr std::string_view format_str{" {}"};
  static constexpr int64_t arg = -8978612134175239201;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);

    REQUIRE(snprintf(buf.data(), buf.size(), " %" PRIi64, arg) == static_cast<int>(emio_str.size()));
    REQUIRE(emio_str == buf.data());
    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    return snprintf(buf.data(), buf.size(), " %" PRIi64, arg);
  };
}

TEST_CASE("format big hex") {
  static constexpr std::string_view format_str{"{:x}"};
  static constexpr uint64_t arg = 8978612134175239201;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);

    REQUIRE(snprintf(buf.data(), buf.size(), "%" PRIx64, arg) == static_cast<int>(emio_str.size()));
    REQUIRE(emio_str == buf.data());
    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    return snprintf(buf.data(), buf.size(), "%" PRIx64, arg);
  };
}

TEST_CASE("format complex format spec") {
  static constexpr std::string_view format_str{"{0:x^+#20X}"};
  static constexpr int64_t arg = 8978612134175239201;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);
    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
  // No snprintf equivalent.
}

TEST_CASE("format zero as double") {
  static constexpr std::string_view format_str{"{}"};
  static constexpr double arg = 0;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);

    REQUIRE(snprintf(buf.data(), buf.size(), "%g", arg) == static_cast<int>(emio_str.size()));
    REQUIRE(emio_str == buf.data());

    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    return snprintf(buf.data(), buf.size(), "%g", arg);
  };
}

TEST_CASE("format shortest double general") {
  static constexpr std::string_view format_str{"{}"};
  static constexpr double arg = M_PI;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);
    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
}

TEST_CASE("format double exponent") {
  static constexpr std::string_view format_str{"{:e}"};
  static constexpr double arg = M_PI;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);

    REQUIRE(snprintf(buf.data(), buf.size(), "%e", arg) == static_cast<int>(emio_str.size()));
    REQUIRE(emio_str == buf.data());

    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    return snprintf(buf.data(), buf.size(), "%e", arg);
  };
}

TEST_CASE("format double fixed") {
  static constexpr std::string_view format_str{"{:f}"};
  static constexpr double arg = M_PI;

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, arg);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, arg);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, arg);
    const std::string fmt_str = fmt::format(format_str, arg);
    REQUIRE(emio_str == fmt_str);

    REQUIRE(snprintf(buf.data(), buf.size(), "%f", arg) == static_cast<int>(emio_str.size()));
    REQUIRE(emio_str == buf.data());

    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), arg).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, arg);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), arg);
  };
  BENCHMARK("snprintf") {
    return snprintf(buf.data(), buf.size(), "%f", arg);
  };
}

TEST_CASE("format many arguments") {
  static constexpr std::string_view format_str{"{} {} {} {} {} {} {} {} {} {}"};
  // No floating-point because this shifts this benchmark result too much because it is much slower in emio than in fmt.
#define ARGS                                                                                                \
  true, static_cast<int8_t>(1), static_cast<uint8_t>(2), static_cast<int16_t>(3), static_cast<uint16_t>(4), \
      static_cast<int32_t>(5), static_cast<uint32_t>(6), "abc", 'x', nullptr

  constexpr size_t emio_formatted_size = emio::formatted_size(format_str, ARGS);
  const size_t fmt_formatted_size = fmt::formatted_size(format_str, ARGS);
  REQUIRE(emio_formatted_size == fmt_formatted_size);
  std::array<char, 2 * emio_formatted_size> buf{};

  BENCHMARK("base") {
    const std::string emio_str = emio::format(format_str, ARGS);
    const std::string fmt_str = fmt::format(format_str, ARGS);
    REQUIRE(emio_str == fmt_str);
    return emio_str == fmt_str;
  };
  BENCHMARK("emio") {
    return emio::format_to(buf.data(), format_str, ARGS);
  };
  BENCHMARK("emio runtime") {
    return emio::format_to(buf.data(), emio::runtime(format_str), ARGS).value();
  };
  BENCHMARK("fmt") {
    return fmt::format_to(buf.data(), format_str, ARGS);
  };
  BENCHMARK("fmt runtime") {
    return fmt::format_to(buf.data(), fmt::runtime(format_str), ARGS);
  };
}
