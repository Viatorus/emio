//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

// This implementation is based on:
// https://github.com/rust-lang/rust/blob/71ef9ecbdedb67c32f074884f503f8e582855c2f/library/core/src/num/flt2dec/decoder.rs

#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace emio::detail::format {

struct finite_result_t {
  uint64_t mant{};
  uint64_t minus{};
  uint64_t plus{};
  int16_t exp{};
  bool inclusive{};
};

enum class category { zero, finite, infinity, nan };

struct decode_result_t {
  bool negative{};
  format::category category{};
  finite_result_t finite{};  // Only valid if category is finite.
};

inline constexpr decode_result_t decode(double value) noexcept {
  decode_result_t res{};

  using bits_type = uint64_t;
  const auto bits = std::bit_cast<bits_type>(value);

  res.negative = bits >> 63 != 0;
  if (value == 0) {
    return res;
  }

  // Exponent bias + mantissa shift
  res.finite.exp = static_cast<int16_t>(((bits >> 52) & 0x7ff) - (1023 + 52));
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
      constexpr auto minnorm = std::bit_cast<bits_type>(std::numeric_limits<double>::min());
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
