// #pragma GCC optimize("O3")

// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <string>

namespace {

void check_emio_vs_fmt(double value) {
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
}

}  // namespace

#define CHECK_EMIO_VS_FMT_SINGLE(x, value) CHECK(emio::format(x, value) == fmt::format(x, value))

TEST_CASE("run") {
  // TODO: precision < 0?
  // {:#.{}e}

  //  SECTION("0.0002") {
  //    double f = GENERATE(take(2, random(0.0001, 0.0002)));
  //    CHECK(fmt::format("{}", f) == emio::format("{}", f));
  //  }
  //  SECTION("1") {
  //    double f = GENERATE(take(2, random(0.1, 1.0)));
  //    CHECK(fmt::format("{}", f) == emio::format("{}", f));
  //  }
  //  SECTION("2") {
  //    double f = GENERATE(take(2, random(1.0, 2.0)));
  //    CHECK(fmt::format("{}", f) == emio::format("{}", f));
  //  }
  //  SECTION("big") {
  //    double f = GENERATE(take(10000, random(-100000000000000000000.0, 100000000000000000000.0)));
  //    CHECK(fmt::format("{:#.16e}", f) == emio::format("{}", f));
  //  }

  //  double f = 1;
  //  CHECK(fmt::format("{:e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:.0e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:#e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:#.0e}", f) == emio::format("{}", f));
  //
  //  f = 1.5e212;
  //  CHECK(fmt::format("{:025}", -23) == emio::format("{:025}", -23));
  //  CHECK(fmt::format("{:0e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:+025e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:.0e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:#e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:#.0e}", f) == emio::format("{}", f));

  //  f = -1.5e-212;
  //  CHECK(fmt::format("{:0e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:.0e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:#e}", f) == emio::format("{}", f));
  //  CHECK(fmt::format("{:#.0e}", f) == emio::format("{}", f));
  // -9.87697380433965942e+304 1
  //
  //  {
  //    namespace emiod = emio::detail::format;
  //    emio::string_buffer<char> buf;
  //
  //    auto d = emiod::decode(9.9999980433965942e-9);
  //    emiod::format_exact(d.finite, buf, emiod::format_exact_mode::significant_digits, 6);
  //    emiod::format_exact(d.finite, buf, emiod::format_exact_mode::significant_digits, 7);
  ////
  ////    d = emiod::decode(0.987697380433965942);
  ////    emiod::format_exact(d.finite, buf, emiod::format_exact_mode::decimal_point, 1);
  ////    emiod::format_exact(d.finite, buf, emiod::format_exact_mode::decimal_point, 0);
  ////
  ////    d = emiod::decode(0.0987697380433965942);
  ////    emiod::format_exact(d.finite, buf, emiod::format_exact_mode::decimal_point, 1);
  ////    emiod::format_exact(d.finite, buf, emiod::format_exact_mode::decimal_point, 0);
  //
  //    return;
  //  }

  //  double d = 1e-4;
  //  WARN(fmt::format("{:f} {:f} {:f} {:.0f}", 1.2, 1e-5, 23e14, 95.45));
  //  WARN(fmt::format("{:.1f} {:.50f} {:g} {:#.0f}", 1.2, 1e-5, 23e14, 95.45));
  //  WARN(fmt::format("{:} {:} {:} {:}", 1.2, 1e-5, 23e14, 95.45));
  //  d = std::nexttoward(d, 1e-5);
  //  WARN(fmt::format("{:.35g}", d));
  //  WARN(fmt::format("{:.17g}", d));
  //  WARN(fmt::format("{:.16g}", d));
  //  WARN(fmt::format("{:.15g}", d));
  //  WARN(fmt::format("{:.14g}", d));
  //  // 9.9999999999999991239646446317124173e-05
  //  return;

  //  WARN(emio::format("{} {} {} {} {}", 1.456, 12.458, 0.031897849875589732, 1e15, 23e14));
  // Got:
  //

  {
//    double d = 1234567890987653.3;
//    double p = std::nexttoward(d, -INFINITY);
//    double n = std::nexttoward(d, INFINITY);

    //    WARN(emio::format("{:.10f}", n));
    //    WARN(emio::format("{:.10f}", d));
    //    WARN(emio::format("{:.10f}", p));
    //
    //    WARN(fmt::format("{:.10f}", n));
    //    WARN(fmt::format("{:.10f}", d));
//    WARN(fmt::format("{:#.0e}", -42.0));
//    WARN(emio::format("{:#}", std::numeric_limits<double>::min()));
//    WARN(emio::format("{:f}", 0.0));
  }
//
//  //  CHECK_EMIO_VS_FMT_SINGLE("{:}", std::nexttoward(1234567890987653.21, -INFINITY));
//  //  CHECK_EMIO_VS_FMT_SINGLE("{:}", std::nexttoward(1234567890987653.21, -INFINITY));
//  CHECK_EMIO_VS_FMT_SINGLE("{:#}", -42.0);
//  CHECK_EMIO_VS_FMT_SINGLE("{:#.0f}", -42.0);
//  CHECK_EMIO_VS_FMT_SINGLE("{:#.0e}", -42.0);
//  CHECK_EMIO_VS_FMT_SINGLE("{:#.0g}", -42.0);
  //  CHECK_EMIO_VS_FMT_SINGLE("{:}", std::nexttoward(1234567890987653.21, INFINITY));
  //
  //  return;
  //  CHECK_EMIO_VS_FMT_SINGLE("{:.2}", 12300);
  //
  //  CHECK_EMIO_VS_FMT_SINGLE("{:015e}", -1.456);
  //  CHECK_EMIO_VS_FMT_SINGLE("{: 15e}", -1.456);
  //  CHECK_EMIO_VS_FMT_SINGLE("{:+015e}", -1.456);
  //
  //  CHECK_EMIO_VS_FMT_SINGLE("{:015e}", 1.456);
  //  CHECK_EMIO_VS_FMT_SINGLE("{: 15e}", 1.456);
  //  CHECK_EMIO_VS_FMT_SINGLE("{:+015e}", 1.456);
  //
  //  CHECK_EMIO_VS_FMT_SINGLE("{:+15e}", INFINITY);
  //  CHECK_EMIO_VS_FMT_SINGLE("{:015e}", INFINITY);
  //  CHECK_EMIO_VS_FMT_SINGLE("{:+015e}", INFINITY);

  // Shortest of: 1234567890987653.25 -> 1234567890987653.2 (fmt) != 1234567890987653.3 (emio/rust)

  static constexpr std::array values = {
      0.0,
      1.0,
      12.0,
      123.0,
      1230.0,
      12300.0,
      123456789098765.0,
      1234567890987653.0,
      1.2,
      12.34,
      123.56,
      123456789098765.5,
      1234567890987653.9,
      0.1,
      0.012,
      0.00123,
      0.00000123,
      std::numeric_limits<double>::min(),
      std::numeric_limits<double>::max(),
      std::numeric_limits<double>::lowest(),
//      std::numeric_limits<double>::infinity(),
//      std::numeric_limits<double>::quiet_NaN(),
//      std::numeric_limits<double>::signaling_NaN(),
      M_PI,
  };

  for (double value : values) {
    check_emio_vs_fmt(value);
  }

  return;
  // double lower_bound = 0;
  //   double upper_bound = std::numeric_limits<double>::max() / 2;
  ////   std::student_t_distribution<double> unif{15};
  ////   std::default_random_engine re;
  // std::random_device rd;  // Will be used to obtain a seed for the random number engine
  //     std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  //     std::uniform_real_distribution<> dis(lower_bound, upper_bound);
  uint64_t lower_bound = 0;
  uint64_t upper_bound = std::numeric_limits<uint64_t>::max();
  //   std::student_t_distribution<double> unif{15};
  //   std::default_random_engine re;
  std::random_device rd;   // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd());  // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint64_t> dis(lower_bound, upper_bound);

  while (true) {
    // double a_random_double = dis(gen);
    double d{};
    uint64_t x = dis(gen);
    memcpy(&d, &x, 8);
    check_emio_vs_fmt(d);
  }

  //  CHECK_EMIO_FMT("{}", 1.0);
  //  CHECK_EMIO_FMT("{}", -0.0);
  //  CHECK_EMIO_FMT("{}", 0.0);
  //  CHECK_EMIO_FMT("{}", 12.0);
  //  CHECK_EMIO_FMT("{}", 123.0);
  //  CHECK_EMIO_FMT("{}", 1230.0);
  //  CHECK_EMIO_FMT("{}", 12300.0);
  //  CHECK_EMIO_FMT("{}", 1.2);
  //  CHECK_EMIO_FMT("{}", 12.34);
  //  CHECK_EMIO_FMT("{}", 123.56);
  //  CHECK_EMIO_FMT("{}", 0.1);
  //  CHECK_EMIO_FMT("{}", 0.12);
  //  CHECK_EMIO_FMT("{}", 0.012);
  //  CHECK_EMIO_FMT("{}", 0.00123);
  //
  //  CHECK_EMIO_FMT("{:e}", -0.0);
  //  CHECK_EMIO_FMT("{:e}", 0.0);
  //  CHECK_EMIO_FMT("{:e}", 1.0);
  //  CHECK_EMIO_FMT("{:e}", 12.0);
  //  CHECK_EMIO_FMT("{:e}", 123.0);
  //  CHECK_EMIO_FMT("{:e}", 1230.0);
  //  CHECK_EMIO_FMT("{:e}", 12300.0);
  //  CHECK_EMIO_FMT("{:e}", 1.2);
  //  CHECK_EMIO_FMT("{:e}", 12.34);
  //  CHECK_EMIO_FMT("{:e}", 123.56);
  //  CHECK_EMIO_FMT("{:e}", 0.1);
  //  CHECK_EMIO_FMT("{:e}", 0.12);
  //  CHECK_EMIO_FMT("{:e}", 0.012);
  //  CHECK_EMIO_FMT("{:e}", 0.00123);
  //
  //  CHECK_EMIO_FMT("{:f}", -0.0);
  //  CHECK_EMIO_FMT("{:f}", 0.0);
  //  CHECK_EMIO_FMT("{:f}", 1.0);
  //  CHECK_EMIO_FMT("{:f}", 12.0);
  //  CHECK_EMIO_FMT("{:f}", 123.0);
  //  CHECK_EMIO_FMT("{:f}", 1230.0);
  //  CHECK_EMIO_FMT("{:f}", 12300.0);
  //  CHECK_EMIO_FMT("{:f}", 1.2);
  //  CHECK_EMIO_FMT("{:f}", 12.34);
  //  CHECK_EMIO_FMT("{:f}", 123.56);
  //  CHECK_EMIO_FMT("{:f}", 0.1);
  //  CHECK_EMIO_FMT("{:f}", 0.12);
  //  CHECK_EMIO_FMT("{:f}", 0.012);
  //  CHECK_EMIO_FMT("{:f}", 0.00123);

  //  WARN(emio::format("{} {:}", lng, 1e-5));
  //  WARN(emio::format("{} {:}", lng, 23e14));
  //  WARN(emio::format("{} {:}", lng, 95.45));
  //  WARN(emio::format("{:} {:}", 1, 23e14));
  //  WARN(emio::format("{:}", 95.45));
  // Got: 1.456000e+07 1.2458000e+09 3.1898e+03\x00 1.000000000000000000000e+37 2.300000000000000000000e+37
  // 1.456000 12.458000 0.031898 1000000000000000.000000 2300000000000000.000000
  return;

  WARN(emio::format("{:g} {:g} {:g} {:g} {:g}", 1.456, 12.458, 0.031897849875589732, 1e15, 23e14));
  WARN(emio::format("1                           {}", -0.05048));
  WARN(emio::format("1                           |{}|", 0.31589784987));
  WARN(emio::format("123456789abcdefgh{}", "ijklmnopqrstuvw"));
  WARN(emio::format("123456789abcdefgh"));
  WARN(emio::format("1                           {}", 0.2));
  WARN(emio::format("1                           {}", 1.0));

  WARN(emio::format("{0:B>6} {1} {2} {0:F} {1:G} {2:F}", std::numeric_limits<float>::quiet_NaN(),
                    -std::numeric_limits<float>::infinity(), +std::numeric_limits<double>::infinity()));

  //  int g = 0;
  //  double f = 0.499999176;
  //  // for (int i = 0; i < 500000; i++) {
  //  while (true) {
  //    namespace policy = jkj::dragonbox::policy;
  //
  //    f = std::nextafter(f, std::numeric_limits<float>::infinity());
  //    const auto result = jkj::dragonbox::to_decimal(f, policy::cache::compact, policy::trailing_zero::remove);
  //    //    INFO(emio::format("{}", f));
  //    if (rrr == true) {
  //      float p = std::nextafter(f, -std::numeric_limits<float>::infinity());
  //      float n = std::nextafter(f, std::numeric_limits<float>::infinity());
  //
  //      WARN(fmt::format("{0:} {1:} {2:} - {0:.9e} {1:.9e} {2:.9e} - {3}", p, f, n, rrr));
  //
  ////      CHECK(p == -1.0);
  ////      CHECK(f == -1.0);
  ////      CHECK(n == -1.0);
  //      rrr = false;
  //      if (g++ > 500) {
  //        break;
  //      }
  //    } else {
  //        rrr = false;
  //    };
  //  }

  //  double g = std::nextafter(f, 1.0);
  //  WARN(emio::format("{}", g));

  //  CHECK(f != g);

  //  CHECK(emio::format(emio::runtime("{{{0}}}"), 42) == "{42}");
  //  WARN(d2s(d));
  //  WARN(d2exp(d, 1));
  //  WARN(d2fixed(d, 1));
}
