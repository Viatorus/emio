// Unit under test.
#include "emio/detail/format/decode.hpp"

// Other includes.
#include <limits>

#include "catch2/catch_test_macros.hpp"

TEST_CASE("decode") {
  SECTION("nan") {
    double value = std::numeric_limits<double>::quiet_NaN();
    emiod::decoded_result res = emiod::decode(value);
    CHECK(res.category == emiod::category::nan);

    value = std::numeric_limits<double>::signaling_NaN();
    res = emiod::decode(value);
    CHECK(res.category == emiod::category::nan);
  }

  SECTION("infinity") {
    double value = std::numeric_limits<double>::infinity();
    emiod::decoded_result res = emiod::decode(value);
    CHECK(res.category == emiod::category::infinity);
    CHECK(res.negative == false);

    value = -std::numeric_limits<double>::infinity();
    res = emiod::decode(value);
    CHECK(res.category == emiod::category::infinity);
    CHECK(res.negative == true);
  }
  SECTION("denormalized") {
    double value = std::numeric_limits<double>::denorm_min();
    emiod::decoded_result res = emiod::decode(value);
    CHECK(res.category == emiod::category::finite);
    //    CHECK(res.finite.exp == 0);
  }

  SECTION("normalized") {
    double value = std::numeric_limits<double>::min();
    emiod::decoded_result res = emiod::decode(value);
    CHECK(res.category == emiod::category::finite);
    //    CHECK(res.finite.exp == 0);
  }
}
