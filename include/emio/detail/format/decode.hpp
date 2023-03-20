#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace emio::detail::format {

struct decoded {
  uint64_t mant{};
  uint64_t minus{};
  uint64_t plus{};
  int16_t exp{};
  bool inclusive{};
};

enum class category { zero, finite, infinity, nan };

struct decoded_result {
  bool negative{};
  format::category category{};
  decoded finite{};
};

// template <typename T>
inline constexpr decoded_result decode(double value) {
  using T = double;
  decoded_result res{};

  using bits_type = std::conditional_t<sizeof(T) == sizeof(float), uint32_t, uint64_t>;
  bits_type bits = std::bit_cast<bits_type>(value);

  res.negative = bits >> 63 != 0;
  if (value == 0) {
    return res;
  }

  // Exponent bias + mantissa shift
  res.finite.exp = static_cast<int16_t>((bits >> 52) & 0x7ff) - (1023 + 52);
  res.finite.mant = res.finite.exp == -1075 ? (bits & 0xfffffffffffff) << 1 : (bits & 0xfffffffffffff);
  res.finite.inclusive = (res.finite.mant & 1) == 0;

  if (res.finite.exp == 972) {  // non-numbers.
    if (res.finite.mant == 0) {
      res.category = category::infinity;
    } else {
      res.category = category::nan;
    }
  } else {
    res.category = category::finite;
    res.finite.minus = 1;
    res.finite.plus = 1;
    if (res.finite.exp != -1075) {  // Norm.
      res.finite.mant |= 0x10000000000000;
      constexpr auto minnorm = std::bit_cast<bits_type>(std::numeric_limits<T>::min());
      if (res.finite.mant == minnorm) {
        res.finite.plus = 2;
        res.finite.mant <<= 2;
        res.finite.exp -= 2;
      } else {
        res.finite.mant <<= 1;
        res.finite.exp -= 1;
      }
    }
  }

  return res;
}

}  // namespace emio::detail::format
