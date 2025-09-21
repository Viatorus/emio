//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <string_view>

#include "../../result.hpp"
#include "../bignum.hpp"
#include "../conversion.hpp"

namespace emio::detail::scan {

/// Represents a parsed decimal number before conversion to float
struct decimal {
  int64_t exponent;
  uint64_t mantissa;
  bool negative;
  bool many_digits;
};

struct DecimalSeq {
  static constexpr size_t MAX_DIGITS = 768;
  static constexpr size_t MAX_DIGITS_WITHOUT_OVERFLOW = 19;
  static constexpr int32_t DECIMAL_POINT_RANGE = 2047;

  /// The number of significant digits in the decimal.
  size_t num_digits;
  /// The offset of the decimal point in the significant digits.
  int32_t decimal_point;
  /// If the number of significant digits stored in the decimal is truncated.
  bool truncated;
  /// Buffer of the raw digits, in the range [0, 9].
  std::array<uint8_t, MAX_DIGITS> digits;

  /// Trim trailing zeros from the buffer.
  // FIXME(tgross35): this could be `.rev().position()` if perf is okay
  void trim() {
    // All of the following calls to `DecimalSeq::trim` can't panic because:
    //
    //  1. `parse_decimal` sets `num_digits` to a max of `DecimalSeq::MAX_DIGITS`.
    //  2. `right_shift` sets `num_digits` to `write_index`, which is bounded by `num_digits`.
    //  3. `left_shift` `num_digits` to a max of `DecimalSeq::MAX_DIGITS`.
    //
    // Trim is only called in `right_shift` and `left_shift`.
    EMIO_Z_DEV_ASSERT(num_digits <= MAX_DIGITS);
    while (num_digits != 0 && digits[num_digits - 1] == 0) {
      num_digits -= 1;
    }
  }

  [[nodiscard]] uint64_t round() const {
    if (num_digits == 0 || decimal_point < 0) {
      return 0;
    } else if (decimal_point >= static_cast<int32_t>(MAX_DIGITS_WITHOUT_OVERFLOW)) {
      return 0xFFFFFFFFFFFFFFFFULL;
    }

    size_t dp = static_cast<size_t>(decimal_point);
    uint64_t n = 0;

    for (size_t i = 0; i < dp; ++i) {
      n *= 10;
      if (i < num_digits) {
        n += static_cast<uint64_t>(digits[i]);
      }
    }

    bool round_up = false;

    if (dp < num_digits) {
      round_up = digits[dp] >= 5;
      if (digits[dp] == 5 && dp + 1 == num_digits) {
        round_up = truncated || ((dp != 0) && ((1 & digits[dp - 1]) != 0));
      }
    }

    if (round_up) {
      n += 1;
    }
    return n;
  }

  static size_t number_of_digits_decimal_left_shift(const DecimalSeq& d, size_t shift) {
    static constexpr uint16_t TABLE[65] = {
        0x0000, 0x0800, 0x0801, 0x0803, 0x1006, 0x1009, 0x100D, 0x1812, 0x1817, 0x181D, 0x2024, 0x202B, 0x2033,
        0x203C, 0x2846, 0x2850, 0x285B, 0x3067, 0x3073, 0x3080, 0x388E, 0x389C, 0x38AB, 0x38BB, 0x40CC, 0x40DD,
        0x40EF, 0x4902, 0x4915, 0x4929, 0x513E, 0x5153, 0x5169, 0x5180, 0x5998, 0x59B0, 0x59C9, 0x61E3, 0x61FD,
        0x6218, 0x6A34, 0x6A50, 0x6A6D, 0x6A8B, 0x72AA, 0x72C9, 0x72E9, 0x7B0A, 0x7B2B, 0x7B4D, 0x8370, 0x8393,
        0x83B7, 0x83DC, 0x8C02, 0x8C28, 0x8C4F, 0x9477, 0x949F, 0x94C8, 0x9CF2, 0x051C, 0x051C, 0x051C, 0x051C,
    };
    static constexpr uint8_t TABLE_POW5[0x051C] = {
        5, 2, 5, 1, 2, 5, 6, 2, 5, 3, 1, 2, 5, 1, 5, 6, 2, 5, 7, 8, 1, 2, 5, 3, 9, 0, 6, 2, 5, 1, 9, 5, 3, 1, 2, 5, 9,
        7, 6, 5, 6, 2, 5, 4, 8, 8, 2, 8, 1, 2, 5, 2, 4, 4, 1, 4, 0, 6, 2, 5, 1, 2, 2, 0, 7, 0, 3, 1, 2, 5, 6, 1, 0, 3,
        5, 1, 5, 6, 2, 5, 3, 0, 5, 1, 7, 5, 7, 8, 1, 2, 5, 1, 5, 2, 5, 8, 7, 8, 9, 0, 6, 2, 5, 7, 6, 2, 9, 3, 9, 4, 5,
        3, 1, 2, 5, 3, 8, 1, 4, 6, 9, 7, 2, 6, 5, 6, 2, 5, 1, 9, 0, 7, 3, 4, 8, 6, 3, 2, 8, 1, 2, 5, 9, 5, 3, 6, 7, 4,
        3, 1, 6, 4, 0, 6, 2, 5, 4, 7, 6, 8, 3, 7, 1, 5, 8, 2, 0, 3, 1, 2, 5, 2, 3, 8, 4, 1, 8, 5, 7, 9, 1, 0, 1, 5, 6,
        2, 5, 1, 1, 9, 2, 0, 9, 2, 8, 9, 5, 5, 0, 7, 8, 1, 2, 5, 5, 9, 6, 0, 4, 6, 4, 4, 7, 7, 5, 3, 9, 0, 6, 2, 5, 2,
        9, 8, 0, 2, 3, 2, 2, 3, 8, 7, 6, 9, 5, 3, 1, 2, 5, 1, 4, 9, 0, 1, 1, 6, 1, 1, 9, 3, 8, 4, 7, 6, 5, 6, 2, 5, 7,
        4, 5, 0, 5, 8, 0, 5, 9, 6, 9, 2, 3, 8, 2, 8, 1, 2, 5, 3, 7, 2, 5, 2, 9, 0, 2, 9, 8, 4, 6, 1, 9, 1, 4, 0, 6, 2,
        5, 1, 8, 6, 2, 6, 4, 5, 1, 4, 9, 2, 3, 0, 9, 5, 7, 0, 3, 1, 2, 5, 9, 3, 1, 3, 2, 2, 5, 7, 4, 6, 1, 5, 4, 7, 8,
        5, 1, 5, 6, 2, 5, 4, 6, 5, 6, 6, 1, 2, 8, 7, 3, 0, 7, 7, 3, 9, 2, 5, 7, 8, 1, 2, 5, 2, 3, 2, 8, 3, 0, 6, 4, 3,
        6, 5, 3, 8, 6, 9, 6, 2, 8, 9, 0, 6, 2, 5, 1, 1, 6, 4, 1, 5, 3, 2, 1, 8, 2, 6, 9, 3, 4, 8, 1, 4, 4, 5, 3, 1, 2,
        5, 5, 8, 2, 0, 7, 6, 6, 0, 9, 1, 3, 4, 6, 7, 4, 0, 7, 2, 2, 6, 5, 6, 2, 5, 2, 9, 1, 0, 3, 8, 3, 0, 4, 5, 6, 7,
        3, 3, 7, 0, 3, 6, 1, 3, 2, 8, 1, 2, 5, 1, 4, 5, 5, 1, 9, 1, 5, 2, 2, 8, 3, 6, 6, 8, 5, 1, 8, 0, 6, 6, 4, 0, 6,
        2, 5, 7, 2, 7, 5, 9, 5, 7, 6, 1, 4, 1, 8, 3, 4, 2, 5, 9, 0, 3, 3, 2, 0, 3, 1, 2, 5, 3, 6, 3, 7, 9, 7, 8, 8, 0,
        7, 0, 9, 1, 7, 1, 2, 9, 5, 1, 6, 6, 0, 1, 5, 6, 2, 5, 1, 8, 1, 8, 9, 8, 9, 4, 0, 3, 5, 4, 5, 8, 5, 6, 4, 7, 5,
        8, 3, 0, 0, 7, 8, 1, 2, 5, 9, 0, 9, 4, 9, 4, 7, 0, 1, 7, 7, 2, 9, 2, 8, 2, 3, 7, 9, 1, 5, 0, 3, 9, 0, 6, 2, 5,
        4, 5, 4, 7, 4, 7, 3, 5, 0, 8, 8, 6, 4, 6, 4, 1, 1, 8, 9, 5, 7, 5, 1, 9, 5, 3, 1, 2, 5, 2, 2, 7, 3, 7, 3, 6, 7,
        5, 4, 4, 3, 2, 3, 2, 0, 5, 9, 4, 7, 8, 7, 5, 9, 7, 6, 5, 6, 2, 5, 1, 1, 3, 6, 8, 6, 8, 3, 7, 7, 2, 1, 6, 1, 6,
        0, 2, 9, 7, 3, 9, 3, 7, 9, 8, 8, 2, 8, 1, 2, 5, 5, 6, 8, 4, 3, 4, 1, 8, 8, 6, 0, 8, 0, 8, 0, 1, 4, 8, 6, 9, 6,
        8, 9, 9, 4, 1, 4, 0, 6, 2, 5, 2, 8, 4, 2, 1, 7, 0, 9, 4, 3, 0, 4, 0, 4, 0, 0, 7, 4, 3, 4, 8, 4, 4, 9, 7, 0, 7,
        0, 3, 1, 2, 5, 1, 4, 2, 1, 0, 8, 5, 4, 7, 1, 5, 2, 0, 2, 0, 0, 3, 7, 1, 7, 4, 2, 2, 4, 8, 5, 3, 5, 1, 5, 6, 2,
        5, 7, 1, 0, 5, 4, 2, 7, 3, 5, 7, 6, 0, 1, 0, 0, 1, 8, 5, 8, 7, 1, 1, 2, 4, 2, 6, 7, 5, 7, 8, 1, 2, 5, 3, 5, 5,
        2, 7, 1, 3, 6, 7, 8, 8, 0, 0, 5, 0, 0, 9, 2, 9, 3, 5, 5, 6, 2, 1, 3, 3, 7, 8, 9, 0, 6, 2, 5, 1, 7, 7, 6, 3, 5,
        6, 8, 3, 9, 4, 0, 0, 2, 5, 0, 4, 6, 4, 6, 7, 7, 8, 1, 0, 6, 6, 8, 9, 4, 5, 3, 1, 2, 5, 8, 8, 8, 1, 7, 8, 4, 1,
        9, 7, 0, 0, 1, 2, 5, 2, 3, 2, 3, 3, 8, 9, 0, 5, 3, 3, 4, 4, 7, 2, 6, 5, 6, 2, 5, 4, 4, 4, 0, 8, 9, 2, 0, 9, 8,
        5, 0, 0, 6, 2, 6, 1, 6, 1, 6, 9, 4, 5, 2, 6, 6, 7, 2, 3, 6, 3, 2, 8, 1, 2, 5, 2, 2, 2, 0, 4, 4, 6, 0, 4, 9, 2,
        5, 0, 3, 1, 3, 0, 8, 0, 8, 4, 7, 2, 6, 3, 3, 3, 6, 1, 8, 1, 6, 4, 0, 6, 2, 5, 1, 1, 1, 0, 2, 2, 3, 0, 2, 4, 6,
        2, 5, 1, 5, 6, 5, 4, 0, 4, 2, 3, 6, 3, 1, 6, 6, 8, 0, 9, 0, 8, 2, 0, 3, 1, 2, 5, 5, 5, 5, 1, 1, 1, 5, 1, 2, 3,
        1, 2, 5, 7, 8, 2, 7, 0, 2, 1, 1, 8, 1, 5, 8, 3, 4, 0, 4, 5, 4, 1, 0, 1, 5, 6, 2, 5, 2, 7, 7, 5, 5, 5, 7, 5, 6,
        1, 5, 6, 2, 8, 9, 1, 3, 5, 1, 0, 5, 9, 0, 7, 9, 1, 7, 0, 2, 2, 7, 0, 5, 0, 7, 8, 1, 2, 5, 1, 3, 8, 7, 7, 7, 8,
        7, 8, 0, 7, 8, 1, 4, 4, 5, 6, 7, 5, 5, 2, 9, 5, 3, 9, 5, 8, 5, 1, 1, 3, 5, 2, 5, 3, 9, 0, 6, 2, 5, 6, 9, 3, 8,
        8, 9, 3, 9, 0, 3, 9, 0, 7, 2, 2, 8, 3, 7, 7, 6, 4, 7, 6, 9, 7, 9, 2, 5, 5, 6, 7, 6, 2, 6, 9, 5, 3, 1, 2, 5, 3,
        4, 6, 9, 4, 4, 6, 9, 5, 1, 9, 5, 3, 6, 1, 4, 1, 8, 8, 8, 2, 3, 8, 4, 8, 9, 6, 2, 7, 8, 3, 8, 1, 3, 4, 7, 6, 5,
        6, 2, 5, 1, 7, 3, 4, 7, 2, 3, 4, 7, 5, 9, 7, 6, 8, 0, 7, 0, 9, 4, 4, 1, 1, 9, 2, 4, 4, 8, 1, 3, 9, 1, 9, 0, 6,
        7, 3, 8, 2, 8, 1, 2, 5, 8, 6, 7, 3, 6, 1, 7, 3, 7, 9, 8, 8, 4, 0, 3, 5, 4, 7, 2, 0, 5, 9, 6, 2, 2, 4, 0, 6, 9,
        5, 9, 5, 3, 3, 6, 9, 1, 4, 0, 6, 2, 5,
    };

    shift &= 63;
    uint16_t x_a = TABLE[shift];
    uint16_t x_b = TABLE[shift + 1];
    size_t num_new_digits = static_cast<size_t>(x_a >> 11);
    size_t pow5_a = static_cast<size_t>(0x7FF & x_a);
    size_t pow5_b = static_cast<size_t>(0x7FF & x_b);
    const uint8_t* pow5 = &TABLE_POW5[pow5_a];

    for (size_t i = 0; i < pow5_b - pow5_a; ++i) {
      uint8_t p5 = pow5[i];
      if (i >= d.num_digits) {
        return num_new_digits - 1;
      } else if (d.digits[i] == p5) {
        continue;
      } else if (d.digits[i] < p5) {
        return num_new_digits - 1;
      } else {
        return num_new_digits;
      }
    }

    return num_new_digits;
  }

  /// Computes decimal * 2^shift.
  void left_shift(size_t shift) {
    if (num_digits == 0) {
      return;
    }
    size_t num_new_digits = number_of_digits_decimal_left_shift(*this, shift);
    size_t read_index = num_digits;
    size_t write_index = num_digits + num_new_digits;
    uint64_t n = 0;

    while (read_index != 0) {
      read_index -= 1;
      write_index -= 1;
      n += (static_cast<uint64_t>(digits[read_index]) << shift);
      uint64_t quotient = n / 10;
      uint64_t remainder = n - (10 * quotient);
      if (write_index < MAX_DIGITS) {
        digits[write_index] = static_cast<uint8_t>(remainder);
      } else if (remainder > 0) {
        truncated = true;
      }
      n = quotient;
    }

    while (n > 0) {
      write_index -= 1;
      uint64_t quotient = n / 10;
      uint64_t remainder = n - (10 * quotient);
      if (write_index < MAX_DIGITS) {
        digits[write_index] = static_cast<uint8_t>(remainder);
      } else if (remainder > 0) {
        truncated = true;
      }
      n = quotient;
    }

    num_digits += num_new_digits;

    if (num_digits > MAX_DIGITS) {
      num_digits = MAX_DIGITS;
    }

    decimal_point += static_cast<int32_t>(num_new_digits);
    trim();
  }

  /// Computes decimal * 2^-shift.
  void right_shift(size_t shift) {
    size_t read_index = 0;
    size_t write_index = 0;
    uint64_t n = 0;
    while ((n >> shift) == 0) {
      if (read_index < num_digits) {
        n = (10 * n) + static_cast<uint64_t>(digits[read_index]);
        ++read_index;
      } else if (n == 0) {
        return;
      } else {
        while ((n >> shift) == 0) {
          n *= 10;
          ++read_index;
        }
        break;
      }
    }
    decimal_point -= static_cast<int32_t>(read_index) - 1;
    if (decimal_point < -static_cast<int32_t>(DECIMAL_POINT_RANGE)) {
      // Reset to default, but without clearing digits.
      num_digits = 0;
      decimal_point = 0;
      truncated = false;
      return;
    }
    uint64_t mask = (static_cast<uint64_t>(1) << shift) - 1;
    while (read_index < num_digits) {
      uint8_t new_digit = static_cast<uint8_t>(n >> shift);
      n = (10 * (n & mask)) + static_cast<uint64_t>(digits[read_index]);
      ++read_index;
      digits[write_index] = new_digit;
      ++write_index;
    }
    while (n > 0) {
      uint8_t new_digit = static_cast<uint8_t>(n >> shift);
      n = 10 * (n & mask);
      if (write_index < MAX_DIGITS) {
        digits[write_index] = new_digit;
        ++write_index;
      } else if (new_digit > 0) {
        truncated = true;
      }
    }
    num_digits = write_index;
    trim();
  }
};

struct BiasedFp {
  uint64_t mantissa;
  int32_t power2;

  static constexpr BiasedFp zero_pow2(int32_t p_biased) noexcept {
    return BiasedFp{.mantissa = 0, .power2 = p_biased};
  }
};

/// Parse a big integer representation of the float as a decimal.
constexpr std::pair<DecimalSeq, size_t> parse_decimal_seq(std::string_view s) noexcept {
  DecimalSeq d{};
  const char* const start = s.data();
  size_t start_len = s.size();

  // Skip leading zeros
  while (!s.empty() && s.front() == '0') {
    s.remove_prefix(1);
  }

  // Parse integer digits before decimal point
  auto parse_digits = [](std::string_view& sv, DecimalSeq& dec) {
    while (!sv.empty() && dec.num_digits < DecimalSeq::MAX_DIGITS && sv.front() >= '0' && sv.front() <= '9') {
      dec.digits[dec.num_digits++] = static_cast<uint8_t>(sv.front() - '0');
      sv.remove_prefix(1);
    }
  };
  parse_digits(s, d);

  // Parse fractional part if present
  if (!s.empty() && s.front() == '.') {
    s.remove_prefix(1);
    // const char* first = s.data(); // TODO
    size_t first_len = s.size();

    // Skip leading zeros after decimal point if no digits yet
    if (d.num_digits == 0) {
      while (!s.empty() && s.front() == '0') {
        s.remove_prefix(1);
      }
    }

    // Parse digits after decimal point
    parse_digits(s, d);

    // Set decimal_point to number of digits after decimal point
    d.decimal_point = static_cast<int32_t>(s.size() - first_len);
  }

  if (d.num_digits != 0) {
    // Ignore the trailing zeros if there are any
    size_t n_trailing_zeros = 0;
    size_t parsed_len = start_len - s.size();
    for (size_t i = 0; i < parsed_len; ++i) {
      char c = start[parsed_len - 1 - i];
      if (c == '0') {
        ++n_trailing_zeros;
      } else if (c != '.') {
        break;
      }
    }
    d.decimal_point += static_cast<int32_t>(n_trailing_zeros);
    if (d.num_digits >= n_trailing_zeros) {
      d.num_digits -= n_trailing_zeros;
    } else {
      d.num_digits = 0;
    }
    d.decimal_point += static_cast<int32_t>(d.num_digits);
    if (d.num_digits > DecimalSeq::MAX_DIGITS) {
      d.truncated = true;
      d.num_digits = DecimalSeq::MAX_DIGITS;
    }
  }

  // Parse exponent if present
  if (!s.empty() && (s.front() == 'e' || s.front() == 'E')) {
    s.remove_prefix(1);
    bool neg_exp = false;
    if (!s.empty() && (s.front() == '-' || s.front() == '+')) {
      neg_exp = (s.front() == '-');
      s.remove_prefix(1);
    }
    int32_t exp_num = 0;
    while (!s.empty() && s.front() >= '0' && s.front() <= '9') {
      if (exp_num < 0x10000) {
        exp_num = 10 * exp_num + static_cast<int32_t>(s.front() - '0');
      }
      s.remove_prefix(1);
    }
    if (neg_exp) {
      d.decimal_point -= exp_num;
    } else {
      d.decimal_point += exp_num;
    }
  }

  // Zero out remaining digits up to MAX_DIGITS_WITHOUT_OVERFLOW
  for (size_t i = d.num_digits; i < DecimalSeq::MAX_DIGITS_WITHOUT_OVERFLOW; ++i) {
    d.digits[i] = 0;
  }

  return std::pair{d, s.data() - start};
}

/// Parse the significant digits and biased, binary exponent of a float.
///
/// This is a fallback algorithm that uses a big-integer representation
/// of the float, and therefore is considerably slower than faster
/// approximations. However, it will always determine how to round
/// the significant digits to the nearest machine float, allowing
/// use to handle near half-way cases.
///
/// Near half-way cases are halfway between two consecutive machine floats.
/// For example, the float `16777217.0` has a bitwise representation of
/// `100000000000000000000000 1`. Rounding to a single-precision float,
/// the trailing `1` is truncated. Using round-nearest, tie-even, any
/// value above `16777217.0` must be rounded up to `16777218.0`, while
/// any value before or equal to `16777217.0` must be rounded down
/// to `16777216.0`. These near-halfway conversions therefore may require
/// a large number of digits to unambiguously determine how to round.
///
/// The algorithms described here are based on "Processing Long Numbers Quickly",
/// available here: <https://arxiv.org/pdf/2101.11408.pdf#section.11>.

static constexpr size_t MAX_SHIFT = 60;
static constexpr size_t NUM_POWERS = 19;
static constexpr std::array<uint8_t, 19> POWERS = {0,  3,  6,  9,  13, 16, 19, 23, 26, 29,
                                                   33, 36, 39, 43, 46, 49, 53, 56, 59};

struct Floater {
  int32_t infinite_power{};
  int32_t exp_min{};
  int32_t sig_bits{};
};

template <typename F>
constexpr Floater get_floater() {
  if constexpr (std::is_same_v<F, float>) {
    return Floater{
        .infinite_power = 255,
        .exp_min = -126,
        .sig_bits = 23,
    };
  } else if constexpr (std::is_same_v<F, double>) {
    return Floater{
        .infinite_power = 2047,
        .exp_min = -1022,
        .sig_bits = 52,
    };
  }
}

constexpr BiasedFp parse_long_mantissa(DecimalSeq d, const Floater f) noexcept {
  auto get_shift = [](size_t n) constexpr -> size_t {
    return (n < NUM_POWERS) ? static_cast<size_t>(POWERS[n]) : MAX_SHIFT;
  };

  const BiasedFp fp_zero = BiasedFp::zero_pow2(0);
  const BiasedFp fp_inf = BiasedFp::zero_pow2(/* F:: */ f.infinite_power);

  // Short-circuit if the value can only be a literal 0 or infinity.
  if (d.num_digits == 0 || d.decimal_point < -324) {
    return fp_zero;
  } else if (d.decimal_point >= 310) {
    return fp_inf;
  }
  int32_t exp2 = 0;
  // Shift right toward (1/2 ... 1].
  while (d.decimal_point > 0) {
    const size_t n = static_cast<size_t>(d.decimal_point);
    const size_t shift = get_shift(n);
    d.right_shift(shift);
    if (d.decimal_point < -DecimalSeq::DECIMAL_POINT_RANGE) {
      return fp_zero;
    }
    exp2 += static_cast<int32_t>(shift);
  }
  // Shift left toward (1/2 ... 1].
  while (d.decimal_point <= 0) {
    size_t shift{};
    if (d.decimal_point == 0) {
      const uint8_t digit = d.digits[0];
      if (digit >= 5) {
        break;
      } else if (digit == 0 || digit == 1) {
        shift = 2;
      } else {
        shift = 1;
      }
    } else {
      shift = get_shift(static_cast<size_t>(-d.decimal_point));
    }
    d.left_shift(shift);
    if (d.decimal_point > DecimalSeq::DECIMAL_POINT_RANGE) {
      return fp_inf;
    }
    exp2 -= static_cast<int32_t>(shift);
  }
  // We are now in the range [1/2 ... 1] but the binary format uses [1 ... 2].
  exp2 -= 1;
  while (f.exp_min > exp2) {
    size_t n = static_cast<size_t>(f.exp_min - exp2);
    n = std::min(n, MAX_SHIFT);
    d.right_shift(n);
    exp2 += static_cast<int32_t>(n);
  }
  if ((exp2 - f.exp_min + 1) >= f.infinite_power) {
    return fp_inf;
  }
  // Shift the decimal to the hidden bit, and then round the value
  // to get the high mantissa+1 bits.
  d.left_shift(static_cast<size_t>(f.sig_bits) + 1);
  uint64_t mantissa = d.round();
  if (mantissa >= (1ULL << (f.sig_bits + 1))) {
    // Rounding up overflowed to the carry bit, need to
    // shift back to the hidden bit.
    d.right_shift(1);
    exp2 += 1;
    mantissa = d.round();
    if ((exp2 - f.exp_min + 1) >= f.infinite_power) {
      return fp_inf;
    }
  }
  int32_t power2 = exp2 - f.exp_min + 1;
  if (mantissa < (1ULL << f.sig_bits)) {
    power2 -= 1;
  }
  // Zero out all the bits above the explicit mantissa bits.
  mantissa &= (1ULL << f.sig_bits) - 1;
  return BiasedFp{mantissa, power2};
}

/// Main float parsing function
constexpr result<std::tuple<BiasedFp, bool, size_t>> parse_float_impl(std::string_view s, const Floater f) noexcept {
  if (s.empty()) {
    return err::eof;
  }

  // Handle sign
  bool negative = false;
  size_t sign_size = 0;
  if (s[0] == '-') {
    negative = true;
    sign_size = 1;
    s = s.substr(1);
  } else if (s[0] == '+') {
    sign_size = 1;
    s = s.substr(1);
  }

  if (s.empty()) {
    return err::eof;
  }

  // Parse the number.
  auto [d, consumed] = parse_decimal_seq(s);
  return std::tuple{parse_long_mantissa(d, f), negative, consumed + sign_size};
}

/// Parse a float from a string view
template <typename Float>
constexpr result<std::pair<Float, size_t>> parse_float(std::string_view s) noexcept {
  auto f = get_floater<Float>();
  EMIO_TRY(auto z, parse_float_impl(s, f));

  uint64_t word = std::get<0>(z).mantissa;
  word |= static_cast<uint64_t>(std::get<0>(z).power2) << f.sig_bits;
  Float result;
  std::memcpy(&result, &word, sizeof(Float));

  if (std::get<1>(z)) {
    result = -result;
  }
  return std::pair{result, std::get<2>(z)};
}

}  // namespace emio::detail::scan