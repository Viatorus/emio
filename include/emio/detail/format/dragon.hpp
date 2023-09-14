//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

// This implementation is based on:
// https://github.com/rust-lang/rust/blob/71ef9ecbdedb67c32f074884f503f8e582855c2f/library/core/src/num/flt2dec/strategy/dragon.rs

#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>

#include "../../buffer.hpp"
#include "../bignum.hpp"
#include "decode.hpp"

namespace emio::detail::format {

inline constexpr int16_t estimate_scaling_factor(uint64_t mant, int16_t exp) noexcept {
  // 2^(nbits-1) < mant <= 2^nbits if mant > 0
  const int nbits = 64 - std::countl_zero(mant - 1);
  // 1292913986 = floor(2^32 * log_10 2)
  // therefore this always underestimates (or is exact), but not much.
  return static_cast<int16_t>((static_cast<int64_t>(nbits + exp) * 1292913986) >> 32);
}

inline constexpr std::optional<char> round_up(std::span<char> d) noexcept {
  const auto end = d.rend();
  auto it = std::find_if(d.rbegin(), end, [](char c) {
    return c != '9';
  });
  if (it != end) {
    // d[i+1..n] is all nines.
    auto i = static_cast<size_t>(std::distance(it, end) - 1);
    d[i] += 1;  // Round up.
    for (size_t j = i + 1; j < d.size(); j++) {
      d[j] = '0';
    }
    return std::nullopt;
  } else if (!d.empty()) {
    // 999..999 rounds to 1000..000 with an increased exponent
    d[0] = '1';
    for (char& c : d.subspan(1)) {
      c = '0';
    }
    return '0';
  }
  // an empty buffer rounds up (a bit strange but reasonable)
  return '1';
}

struct format_fp_result_t {
  std::span<const char> digits;
  int16_t exp;
};

enum class format_exact_mode { significand_digits, decimal_point };

inline constexpr format_fp_result_t format_exact(const finite_result_t& dec, emio::buffer& buf, format_exact_mode mode,
                                                 int16_t number_of_digits) noexcept {
  EMIO_Z_DEV_ASSERT(dec.mant > 0);
  EMIO_Z_DEV_ASSERT(dec.minus > 0);
  EMIO_Z_DEV_ASSERT(dec.plus > 0);

  // estimate `k_0` from original inputs satisfying `10^(k_0-1) < v <= 10^(k_0+1)`.
  int16_t k = estimate_scaling_factor(dec.mant, dec.exp);

  // `v = mant / scale`.
  auto mant = bignum(dec.mant);
  auto scale = bignum(1U);

  size_t s2 = 0;
  size_t s5 = 0;
  size_t m2 = 0;
  size_t m5 = 0;

  if (dec.exp < 0) {
    s2 = static_cast<size_t>(-dec.exp);
  } else {
    m2 += static_cast<size_t>(dec.exp);
  }

  // divide `mant` by `10^k`. now `scale / 10 < mant <= scale * 10`.
  if (k >= 0) {
    s2 += static_cast<size_t>(k);
    s5 += static_cast<size_t>(k);
  } else {
    m2 += static_cast<size_t>(-k);
    m5 += static_cast<size_t>(-k);
  }

  scale.mul_pow5(s5);
  scale.mul_pow2(s2);

  mant.mul_pow5(m5);
  mant.mul_pow2(m2);

  // calculate required buffer size
  size_t len{};
  size_t extra_len{};
  if (mode == format_exact_mode::significand_digits) {
    len = static_cast<size_t>(number_of_digits);
  } else if ((k + number_of_digits) >= 0) {
    len = static_cast<size_t>(k + number_of_digits);
    extra_len = 1;
  }

  // fixup estimation
  // in order to keep the fixed-size bignum, we actually use `mant + floor(plus) >= scale`.
  // we are not actually modifying `scale`, since we can skip the initial multiplication instead.
  // again with the shortest algorithm, `d[0]` can be zero but will be eventually rounded up.
  // if we are working with the last-digit limitation, we need to shorten the buffer
  // before the actual rendering in order to avoid double rounding.
  // note that we have to enlarge the buffer again when rounding up happens!
  if (mant >= scale) {
    k += 1;
    len += extra_len;
  } else {
    mant.mul_small(10);
  }

  auto dst = buf.get_write_area_of(len).value();

  if (len > 0) {
    // cache `(2, 4, 8) * scale` for digit generation.
    bignum scale2 = scale;
    scale2.mul_pow2(1);
    bignum scale4 = scale;
    scale4.mul_pow2(2);
    bignum scale8 = scale;
    scale8.mul_pow2(3);

    for (size_t i = 0; i < len; i++) {
      if (mant.is_zero()) {
        // following digits are all zeroes, we stop here
        // do *not* try to perform rounding! rather, fill remaining digits.
        for (char& c : dst.subspan(i)) {
          c = '0';
        }
        return {dst, k};
      }

      size_t d = 0;
      if (mant >= scale8) {
        mant.sub(scale8);
        d += 8;
      }
      if (mant >= scale4) {
        mant.sub(scale4);
        d += 4;
      }
      if (mant >= scale2) {
        mant.sub(scale2);
        d += 2;
      }
      if (mant >= scale) {
        mant.sub(scale);
        d += 1;
      }
      EMIO_Z_DEV_ASSERT(mant < scale);
      EMIO_Z_DEV_ASSERT(d < 10);
      dst[i] = static_cast<char>('0' + d);
      mant.mul_small(10);
    }
  }

  // rounding up if we stop in the middle of digits
  // if the following digits are exactly 5000..., check the prior digit and try to
  // round to even (i.e., avoid rounding up when the prior digit is even).
  const auto order = mant <=> (scale.mul_small(5));
  if (order == std::strong_ordering::greater ||
      (order == std::strong_ordering::equal && len > 0 && (dst[len - 1] & 1) == 1)) {
    // if rounding up changes the length, the exponent should also change.
    // but we've been requested a fixed number of digits, so do not alter the buffer...
    if (std::optional<char> c = round_up(dst.subspan(0, len))) {
      k += 1;
      // ...unless we've been requested the fixed precision instead.
      // we also need to check that, if the original buffer was empty,
      // the additional digit can only be added when `k == limit` (edge case).
      if (k > -number_of_digits) {
        if (len == 0) {
          dst = buf.get_write_area_of(1).value();
        }

        if (len != 0 && len < dst.size()) {
          return {};
        }

        if (len < dst.size()) {
          dst[len] = *c;
          len += 1;
        }
      }
    }
  }
  return {dst.subspan(0, len), k};
}

inline constexpr format_fp_result_t format_shortest(const finite_result_t& dec, emio::buffer& buf) noexcept {
  // the number `v` to format is known to be:
  // - equal to `mant * 2^exp`;
  // - preceded by `(mant - 2 * minus) * 2^exp` in the original type; and
  // - followed by `(mant + 2 * plus) * 2^exp` in the original type.
  //
  // obviously, `minus` and `plus` cannot be zero. (for infinities, we use out-of-range values.)
  // also we assume that at least one digit is generated, i.e., `mant` cannot be zero too.
  //
  // this also means that any number between `low = (mant - minus) * 2^exp` and
  // `high = (mant + plus) * 2^exp` will map to this exact floating point number,
  // with bounds included when the original mantissa was even (i.e., `!mant_was_odd`).
  EMIO_Z_DEV_ASSERT(dec.mant > 0);
  EMIO_Z_DEV_ASSERT(dec.minus > 0);
  EMIO_Z_DEV_ASSERT(dec.plus > 0);
  //  EMIO_Z_DEV_ASSERT(buf.() >= MAX_SIG_DIGITS);

  // `a.cmp(&b) < rounding` is `if d.inclusive {a <= b} else {a < b}`
  const auto rounding = [&](std::strong_ordering ordering) noexcept {
    if (dec.inclusive) {
      return ordering <= 0;  // NOLINT(modernize-use-nullptr): false positive
    }
    return ordering < 0;  // NOLINT(modernize-use-nullptr): false positive
  };

  // estimate `k_0` from original inputs satisfying `10^(k_0-1) < high <= 10^(k_0+1)`.
  // the tight bound `k` satisfying `10^(k-1) < high <= 10^k` is calculated later.
  int16_t k = estimate_scaling_factor(dec.mant + dec.plus, dec.exp);

  // convert `{mant, plus, minus} * 2^exp` into the fractional form so that:
  // - `v = mant / scale`
  // - `low = (mant - minus) / scale`
  // - `high = (mant + plus) / scale`
  auto mant = bignum(dec.mant);
  auto minus = bignum(dec.minus);
  auto plus = bignum(dec.plus);
  auto scale = bignum(1U);

  size_t s2 = 0;
  size_t s5 = 0;
  size_t m2 = 0;
  size_t m5 = 0;

  if (dec.exp < 0) {
    s2 = static_cast<size_t>(-dec.exp);
  } else {
    m2 += static_cast<size_t>(dec.exp);
  }

  // divide `mant` by `10^k`. now `scale / 10 < mant + plus <= scale * 10`.
  if (k >= 0) {
    s2 += static_cast<size_t>(k);
    s5 += static_cast<size_t>(k);
  } else {
    m2 += static_cast<size_t>(-k);
    m5 += static_cast<size_t>(-k);
  }

  scale.mul_pow5(s5);
  scale.mul_pow2(s2);

  mant.mul_pow5(m5);
  mant.mul_pow2(m2);
  minus.mul_pow5(m5);
  minus.mul_pow2(m2);
  plus.mul_pow5(m5);
  plus.mul_pow2(m2);

  // fixup when `mant + plus > scale` (or `>=`).
  // we are not actually modifying `scale`, since we can skip the initial multiplication instead.
  // now `scale < mant + plus <= scale * 10` and we are ready to generate digits.
  //
  // note that `d[0]` *can* be zero, when `scale - plus < mant < scale`.
  // in this case rounding-up condition (`up` below) will be triggered immediately.
  if (rounding(scale <=> (bignum{mant}.add(plus)))) {
    // equivalent to scaling `scale` by 10
    k += 1;
  } else {
    mant.mul_small(10);
    minus.mul_small(10);
    plus.mul_small(10);
  }

  // cache `(2, 4, 8) * scale` for digit generation.
  bignum scale2 = scale;
  scale2.mul_pow2(1);
  bignum scale4 = scale;
  scale4.mul_pow2(2);
  bignum scale8 = scale;
  scale8.mul_pow2(3);

  auto dst = buf.get_write_area_of(std::numeric_limits<double>::max_digits10).value();

  bool down{};
  bool up{};
  size_t i{};
  while (true) {
    // invariants, where `d[0..n-1]` are digits generated so far:
    // - `v = mant / scale * 10^(k-n-1) + d[0..n-1] * 10^(k-n)`
    // - `v - low = minus / scale * 10^(k-n-1)`
    // - `high - v = plus / scale * 10^(k-n-1)`
    // - `(mant + plus) / scale <= 10` (thus `mant / scale < 10`)
    // where `d[i..j]` is a shorthand for `d[i] * 10^(j-i) + ... + d[j-1] * 10 + d[j]`.

    // generate one digit: `d[n] = floor(mant / scale) < 10`.
    size_t d = 0;
    if (mant >= scale8) {
      mant.sub(scale8);
      d += 8;
    }
    if (mant >= scale4) {
      mant.sub(scale4);
      d += 4;
    }
    if (mant >= scale2) {
      mant.sub(scale2);
      d += 2;
    }
    if (mant >= scale) {
      mant.sub(scale);
      d += 1;
    }
    EMIO_Z_DEV_ASSERT(mant < scale);
    EMIO_Z_DEV_ASSERT(d < 10);
    dst[i] = static_cast<char>('0' + d);
    i += 1;

    // this is a simplified description of the modified Dragon algorithm.
    // many intermediate derivations and completeness arguments are omitted for convenience.
    //
    // start with modified invariants, as we've updated `n`:
    // - `v = mant / scale * 10^(k-n) + d[0..n-1] * 10^(k-n)`
    // - `v - low = minus / scale * 10^(k-n)`
    // - `high - v = plus / scale * 10^(k-n)`
    //
    // assume that `d[0..n-1]` is the shortest representation between `low` and `high`,
    // i.e., `d[0..n-1]` satisfies both of the following but `d[0..n-2]` doesn't:
    // - `low < d[0..n-1] * 10^(k-n) < high` (bijectivity: digits round to `v`); and
    // - `abs(v / 10^(k-n) - d[0..n-1]) <= 1/2` (the last digit is correct).
    //
    // the second condition simplifies to `2 * mant <= scale`.
    // solving invariants in terms of `mant`, `low` and `high` yields
    // a simpler version of the first condition: `-plus < mant < minus`.
    // since `-plus < 0 <= mant`, we have the correct shortest representation
    // when `mant < minus` and `2 * mant <= scale`.
    // (the former becomes `mant <= minus` when the original mantissa is even.)
    //
    // when the second doesn't hold (`2 * mant > scale`), we need to increase the last digit.
    // this is enough for restoring that condition: we already know that
    // the digit generation guarantees `0 <= v / 10^(k-n) - d[0..n-1] < 1`.
    // in this case, the first condition becomes `-plus < mant - scale < minus`.
    // since `mant < scale` after the generation, we have `scale < mant + plus`.
    // (again, this becomes `scale <= mant + plus` when the original mantissa is even.)
    //
    // in short:
    // - stop and round `down` (keep digits as is) when `mant < minus` (or `<=`).
    // - stop and round `up` (increase the last digit) when `scale < mant + plus` (or `<=`).
    // - keep generating otherwise.
    down = rounding(mant <=> (minus));
    up = rounding(scale <=> (bignum{mant}.add(plus)));
    if (down || up) {
      // we have the shortest representation, proceed to the rounding
      break;
    }

    // restore the invariants.
    // this makes the algorithm always terminating: `minus` and `plus` always increases,
    // but `mant` is clipped modulo `scale` and `scale` is fixed.
    mant.mul_small(10);
    minus.mul_small(10);
    plus.mul_small(10);
  }

  // rounding up happens when
  // i) only the rounding-up condition was triggered, or
  // ii) both conditions were triggered and tie breaking prefers rounding up.
  if (up && (!down || mant.mul_pow2(1) >= scale)) {
    // if rounding up changes the length, the exponent should also change.
    // it seems that this condition is very hard to satisfy (possibly impossible),
    // but we are just being safe and consistent here.
    // SAFETY: we initialized that memory above.
    if (std::optional<char> c = round_up(dst.subspan(0, i))) {
      dst[i] = *c;
      i += 1;
      k += 1;
    }
  }
  return {dst.subspan(0, i), k};
}

}  // namespace emio::detail::format
