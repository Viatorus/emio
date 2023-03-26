//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <span>

namespace emio::detail {

struct carrying_add_result_t {
  uint32_t value;
  bool carry;

  friend constexpr bool operator==(const carrying_add_result_t& lhs,
                                   const carrying_add_result_t& rhs) noexcept = default;
};

inline constexpr carrying_add_result_t carrying_add(uint32_t a, uint32_t b, bool carry) {
  const uint32_t v1 = a + b;
  const bool carry1 = v1 < a;
  const uint32_t v2 = v1 + static_cast<uint32_t>(carry);
  const bool carry2 = v2 < v1;
  return {v2, carry1 || carry2};
}

struct borrowing_sub_result_t {
  uint32_t value;
  bool borrow;

  friend constexpr bool operator==(const borrowing_sub_result_t& lhs,
                                   const borrowing_sub_result_t& rhs) noexcept = default;
};

inline constexpr borrowing_sub_result_t borrowing_sub(uint32_t a, uint32_t b, bool borrow) {
  const uint32_t v1 = a - b;
  const bool borrow1 = v1 > a;
  const uint32_t v2 = v1 - static_cast<uint32_t>(borrow);
  const bool borrow2 = v2 > v1;
  return {v2, borrow1 || borrow2};
}

struct carrying_mul_result_t {
  uint32_t value;
  uint32_t carry;

  friend constexpr bool operator==(const carrying_mul_result_t& lhs,
                                   const carrying_mul_result_t& rhs) noexcept = default;
};

inline constexpr carrying_mul_result_t carrying_mul(uint32_t a, uint32_t b, uint32_t carry) {
  const uint64_t v1 = static_cast<uint64_t>(a) * b + carry;
  const auto v2 = static_cast<uint32_t>(v1);
  const auto carry1 = static_cast<uint32_t>(v1 >> 32U);
  return {v2, carry1};
}

// class bignum;
//
// inline std::ostream& operator<<(std::ostream& os, const bignum& b);

/// Stack-allocated arbitrary-precision (up to certain limit) integer.
///
/// This is backed by a fixed-size array of given type ("digit").
/// While the array is not very large (normally some hundred bytes),
/// copying it recklessly may result in the performance hit.
/// Thus this is intentionally not `Copy`.
///
/// All operations available to bignums panic in the case of overflows.
/// The caller is responsible to use large enough bignum types.
class bignum {
 public:
  /// One plus the offset to the maximum "digit" in use.
  /// This does not decrease, so be aware of the computation order.
  /// `base[size..]` should be zero.
  size_t size;
  /// Digits. `[a, b, c, ...]` represents `a + b*2^W + c*2^(2W) + ...`
  /// where `W` is the number of bits in the digit type.
  std::array<uint32_t, 40> base;

  static constexpr bignum from(size_t sz, std::array<uint32_t, 40> b) {
    bignum bn{};
    bn.size = sz;
    bn.base = b;
    return bn;
  }

  constexpr explicit bignum() noexcept : size{1}, base{} {}

  /// Makes a bignum from one digit.
  template <typename T>
    requires(std::is_unsigned_v<T> && sizeof(T) <= sizeof(uint32_t))
  explicit constexpr bignum(T v) noexcept : size{1}, base{{v}} {}

  /// Makes a bignum from `u64` value.
  template <typename T>
    requires(std::is_unsigned_v<T> && sizeof(T) == sizeof(uint64_t))
  explicit constexpr bignum(T v) noexcept : size{1}, base{{static_cast<uint32_t>(v), static_cast<uint32_t>(v >> 32)}} {
    size += static_cast<size_t>(base[1] > 0);
  }

  /// Returns the internal digits as a slice `[a, b, c, ...]` such that the numeric
  /// value is `a + b * 2^W + c * 2^(2W) + ...` where `W` is the number of bits in
  /// the digit type.
  constexpr std::span<uint32_t> digits() noexcept {
    return {base.data(), size};
  }

  /// Returns the `i`-th bit where bit 0 is the least significant one.
  /// In other words, the bit with weight `2^i`.
  [[nodiscard]] constexpr uint8_t get_bit(size_t i) const noexcept {
    const size_t digitbits = 32;
    const auto d = i / digitbits;
    const auto b = i % digitbits;
    return static_cast<uint8_t>((base.at(d) >> b) & 1U);
  }

  /// Returns `true` if the bignum is zero.
  [[nodiscard]] constexpr bool is_zero() const noexcept {
    return std::all_of(base.begin(), base.end(), [](uint32_t v) {
      return v == 0;
    });
  }

  // add
  // add_small
  constexpr bignum& add_small(uint32_t other) noexcept {
    return add_small_at(0, other);
  }

  constexpr bignum& add_small_at(size_t index, uint32_t other) noexcept {
    size_t i = index;
    auto res = carrying_add(base.at(i), other, false);
    base[i] = res.value;
    i += 1;
    for (; res.carry && (i < base.size()); i++) {
      res = carrying_add(base[i], 0, res.carry);
      base[i] = res.value;
    }
    if (res.carry) {
      std::terminate();
    }
    size = i;
    return *this;
  }

  constexpr bignum& add(const bignum& other) noexcept {
    carrying_add_result_t res{0, false};
    size_t i = 0;
    for (; (i < other.size) || (res.carry && (i < base.size())); i++) {
      res = carrying_add(base[i], other.base[i], res.carry);
      base[i] = res.value;
    }
    if (res.carry) {
      std::terminate();
    }
    if (i > size) {
      size = i;
    }
    return *this;
  }

  /// Subtracts `other` from itself and returns its own mutable reference.
  constexpr bignum& sub_small(uint32_t other) noexcept {
    auto res = borrowing_sub(base[0], other, false);
    base[0] = res.value;
    size_t i = 1;
    for (; res.borrow && (i < base.size()); i++) {
      res = borrowing_sub(base[i], 0, res.borrow);
      base[i] = res.value;
    }
    if (res.borrow) {
      std::terminate();
    }
    if (i == size && size != 1) {
      size -= 1;
    }
    return *this;
  }

  /// Subtracts `other` from itself and returns its own mutable reference.
  constexpr bignum& sub(const bignum& other) noexcept {
    if (size < other.size) {
      std::terminate();
    }
    if (size == 0) {
      return *this;
    }
    borrowing_sub_result_t res{0, false};
    for (size_t i = 0; i < size; i++) {
      res = borrowing_sub(base[i], other.base[i], res.borrow);
      base[i] = res.value;
    }
    if (res.borrow) {
      std::terminate();
    }
    do {
      if (base[size - 1] != 0) {
        break;
      }
    } while (--size != 0);
    return *this;
  }

  /// Multiplies itself by a digit-sized `other` and returns its own
  /// mutable reference.
  constexpr bignum& mul_small(uint32_t other) noexcept {
    return muladd_small(other, 0);
  }

  constexpr bignum& muladd_small(uint32_t other, uint32_t carry) noexcept {
    carrying_mul_result_t res{0, carry};
    for (size_t i = 0; i < size; i++) {
      res = carrying_mul(base[i], other, res.carry);
      base[i] = res.value;
    }
    if (res.carry > 0) {
      base.at(size) = res.carry;
      size += 1;
    }
    return *this;
  }

  [[nodiscard]] bignum mul(const bignum& other) const noexcept {
    const auto& bn_max = size > other.size ? *this : other;
    const auto& bn_min = size > other.size ? other : *this;

    bignum prod{};
    for (size_t i = 0; i < bn_min.size; i++) {
      carrying_mul_result_t res{0, 0};
      for (size_t j = 0; j < bn_max.size; j++) {
        res = carrying_mul(bn_min.base[i], bn_max.base[j], res.carry);
        prod.add_small_at(i + j, res.value);
      }
      if (res.carry > 0) {
        prod.add_small_at(i + bn_max.size, res.carry);
      }
    }
    return prod;
  }

  constexpr bignum& mul_digits(std::span<const uint32_t> other) noexcept {
    const auto& bn_max = size > other.size() ? digits() : other;
    const auto& bn_min = size > other.size() ? other : digits();

    bignum prod{};
    for (size_t i = 0; i < bn_min.size(); i++) {
      carrying_mul_result_t res{0, 0};
      for (size_t j = 0; j < bn_max.size(); j++) {
        res = carrying_mul(bn_min[i], bn_max[j], res.carry);
        prod.add_small_at(i + j, res.value);
      }
      if (res.carry > 0) {
        prod.add_small_at(i + bn_max.size(), res.carry);
      }
    }
    *this = prod;
    return *this;
  }

  /// Multiplies itself by `5^e` and returns its own mutable reference.
  constexpr bignum& pow5mul(size_t k) noexcept {
    // Multiply with the largest single-digit power as long as possible.
    while (k >= 13) {
      mul_small(1220703125);
      k -= 13;
    }
    // Stop if nothing left.
    if (k == 0) {
      return *this;
    }
    // Finish off the remainder.
    uint32_t rest_power{5};
    while (--k > 0) {
      rest_power *= 5;
    }
    return mul_small(rest_power);
  }

  /// Divides itself by a digit-sized `other` and returns its own
  /// mutable reference *and* the remainder.
  constexpr uint32_t div_rem_small(uint32_t other) {
    uint64_t borrow = 0;
    for (size_t i = size; i > 0; i--) {
      const uint64_t v = (base[i - 1] + (borrow << 32U));
      const uint64_t res = v / other;
      base[i - 1] = static_cast<uint32_t>(res);
      borrow = v - res * other;
    }
    return static_cast<uint32_t>(borrow);
  }

  /// Multiplies itself by `2^exp` and returns its own mutable reference.
  constexpr bignum& mul_pow2(size_t exp) noexcept {
    const size_t digits = exp / 32;
    const size_t bits = exp % 32;

    if (digits > 0) {
      for (size_t i = size; i > 0; --i) {
        base[i + digits - 1] = base[i - 1];
      }
      for (size_t i = 0; i < digits; i++) {
        base[i] = 0;
      }
      size += digits;
    }
    if (bits > 0) {
      uint32_t overflow = 0;
      size_t i = 0;
      for (; i < size; i++) {
        auto res = static_cast<uint64_t>(base[i]) << bits;
        base[i] = static_cast<uint32_t>(res) + overflow;
        overflow = static_cast<uint32_t>(res >> 32);
      }
      if (overflow > 0) {
        base.at(i) = overflow;
        size += 1;
      }
    }
    return *this;
  }

  [[nodiscard]] constexpr std::strong_ordering operator<=>(const bignum& other) const noexcept {
    if (size > other.size) {
      return std::strong_ordering::greater;
    }
    if (size < other.size) {
      return std::strong_ordering::less;
    }
    for (size_t i = size; i > 0; i--) {
      if (base[i - 1] > other.base[i - 1]) {
        return std::strong_ordering::greater;
      }
      if (base[i - 1] < other.base[i - 1]) {
        return std::strong_ordering::less;
      }
    }
    return std::strong_ordering::equal;
  }

  constexpr bool operator==(const bignum& other) const noexcept = default;
};

// inline std::ostream& operator<<(std::ostream& os, const bignum& b) {
//   os << "size: " << b.size << " base: [";
//   os << b.base[0];
//   for (size_t i = 1; i < b.size && i < b.base.size(); i++) {
//     os << ", " << b.base[i];
//   }
//   os << "]\n";
//   return os;
// }

}  // namespace emio::detail
