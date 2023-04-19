// Unit under test.
#include "emio/detail/format/decode.hpp"

// Other includes.
#include <limits>

#include "catch2/catch_test_macros.hpp"

TEST_CASE("decode") {
  using emio::detail::format::category;
  using emio::detail::format::decode;
  using emio::detail::format::decode_result_t;
  using emio::detail::format::finite_result_t;

  SECTION("nan") {
    double value = std::numeric_limits<double>::quiet_NaN();
    decode_result_t res = decode(value);
    CHECK(res.category == category::nan);

    value = std::numeric_limits<double>::signaling_NaN();
    res = decode(value);
    CHECK(res.category == category::nan);
  }

  SECTION("infinity") {
    double value = std::numeric_limits<double>::infinity();
    decode_result_t res = decode(value);
    CHECK(res.category == category::infinity);
    CHECK(res.negative == false);

    value = -std::numeric_limits<double>::infinity();
    res = decode(value);
    CHECK(res.category == category::infinity);
    CHECK(res.negative == true);
  }
  SECTION("denormalized") {
    double value = std::numeric_limits<double>::denorm_min();
    decode_result_t res = decode(value);
    CHECK(res.category == category::finite);
    finite_result_t finite = res.finite;
    CHECK(finite.exp == -1075);
    CHECK(finite.mant == 2);
    CHECK(finite.minus == 1);
    CHECK(finite.plus == 1);
    CHECK(finite.inclusive == true);
  }

  SECTION("zero") {
    double value = 0.0;
    decode_result_t res = decode(value);
    CHECK(res.category == category::zero);
    CHECK(res.negative == false);

    value = -0.0;
    res = decode(value);
    CHECK(res.category == category::zero);
    CHECK(res.negative == true);
  }

  SECTION("normalized (1)") {
    double value = std::numeric_limits<double>::min();
    decode_result_t res = decode(value);
    CHECK(res.category == category::finite);
    finite_result_t finite = res.finite;
    CHECK(finite.exp == -1076);
    CHECK(finite.mant == 0x40000000000000);
    CHECK(finite.minus == 1);
    CHECK(finite.plus == 2);
    CHECK(finite.inclusive == true);
  }

  SECTION("normalized (2)") {
    double value = std::numeric_limits<double>::max();
    decode_result_t res = decode(value);
    CHECK(res.category == category::finite);
    finite_result_t finite = res.finite;
    CHECK(finite.exp == 970);
    CHECK(finite.mant == 0x3ffffffffffffe);
    CHECK(finite.minus == 1);
    CHECK(finite.plus == 1);
    CHECK(finite.inclusive == false);
  }
}
