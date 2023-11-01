//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>

#include "predef.hpp"

namespace emio::detail {

constexpr bool isalpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

constexpr bool isdigit(char c) {
  return (c >= '0' && c <= '9');
}

constexpr bool is_valid_number_base(const int base) noexcept {
  return base >= 2 && base <= 36;
}

constexpr std::optional<int> char_to_digit(const char c, const int base) noexcept {
  if (c < '0') {
    return std::nullopt;
  }
  int res{};
  if (c >= 'a') {
    res = c - 'a' + 10;
  } else if (c >= 'A') {
    res = c - 'A' + 10;
  } else {
    res = c - '0';
  }
  if (res < base) {
    return res;
  }
  return std::nullopt;
}

constexpr char digit_to_char(const int digit, bool upper) noexcept {
  if (digit >= 10) {
    EMIO_Z_DEV_ASSERT(digit < 36);
    if (upper) {
      return static_cast<char>(static_cast<int>('A') + (digit - 10));
    }
    return static_cast<char>(static_cast<int>('a') + (digit - 10));
  }
  EMIO_Z_DEV_ASSERT(digit < 10);
  return static_cast<char>(static_cast<int>('0') + digit);
}

template <typename T>
  requires(std::is_unsigned_v<T>)
constexpr size_t count_digits_10(T number) noexcept {
  size_t count = 1;
  for (;;) {
    // Integer division is slow so do it for a group of four digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++".
    if (number < 10) {
      return count;
    }
    if (number < 100) {
      return count + 1;
    }
    if (number < 1000) {
      return count + 2;
    }
    if (number < 10000) {
      return count + 3;
    }
    number /= 10000U;
    count += 4;
  }
}

template <size_t Base, typename T>
  requires(std::is_unsigned_v<T>)
constexpr size_t count_digits(T number) noexcept {
  if (number == 0) {
    return 1;
  }

  if constexpr (Base == 10) {
    return count_digits_10(number);
  } else if constexpr (Base == 2) {
    return static_cast<size_t>(std::bit_width(number));
  } else if constexpr (Base == 8) {
    return static_cast<size_t>((std::bit_width(number) + 2) / 3);
  } else if constexpr (Base == 16) {
    return static_cast<size_t>(((std::bit_width(number) + 3) / 4));
  } else {
    size_t digit_cnt{1};
    for (number /= static_cast<T>(Base); number; number /= static_cast<T>(Base)) {
      ++digit_cnt;
    }
    return digit_cnt;
  }
}

template <typename T>
  requires(std::is_unsigned_v<T>)
constexpr size_t get_number_of_digits(T number, int base) noexcept {
  if (number == 0) {
    return 1;
  }
  if (base == 10) {
    return count_digits<10>(number);
  }
  if (base == 16) {
    return count_digits<16>(number);
  }
  if (base == 2) {
    return count_digits<2>(number);
  }
  if (base == 8) {
    return count_digits<8>(number);
  }
  size_t digit_cnt{1};
  for (number /= static_cast<T>(base); number; number /= static_cast<T>(base)) {
    ++digit_cnt;
  }
  return digit_cnt;
}

template <typename T>
constexpr bool is_negative(T value) noexcept {
  if constexpr (std::is_signed_v<T>) {
    return value < 0;
  } else {
    return false;
  }
}

template <typename T>
constexpr int num_bits() noexcept {
  return std::numeric_limits<T>::digits;
}

template <typename T>
using int32_or_64 = std::conditional_t<num_bits<T>() <= 32, int32_t, int64_t>;

template <typename T>
using uint32_or_64 = std::conditional_t<num_bits<T>() <= 32, uint32_t, uint64_t>;

template <typename T>
using upcasted_int_t = std::conditional_t<std::is_signed_v<T>, int32_or_64<T>, uint32_or_64<T>>;

template <typename T>
  requires(std::is_integral_v<T>)
constexpr auto integer_upcast(T integer) noexcept {
  return static_cast<upcasted_int_t<T>>(integer);
}

template <typename T>
constexpr uint32_or_64<T> to_absolute(T number) noexcept {
  if constexpr (std::is_unsigned_v<T>) {
    return number;
  } else {
    if (is_negative(number)) {
      auto abs = static_cast<uint32_or_64<T>>(number);
      abs = T{} - abs;
      return abs;
    }
    return static_cast<uint32_or_64<T>>(number);
  }
}

template <typename T>
constexpr std::make_unsigned_t<T> to_unsigned(T number) noexcept {
  return static_cast<std::make_unsigned_t<T>>(number);
}

template <typename T>
constexpr std::make_signed_t<T> to_signed(T number) noexcept {
  return static_cast<std::make_signed_t<T>>(number);
}

// Converts value in the range [0, 100) to a string.
inline constexpr const char* digits2(size_t value) noexcept {
  // GCC generates slightly better code when value is pointer-size.
  return &"0001020304050607080910111213141516171819"
      "2021222324252627282930313233343536373839"
      "4041424344454647484950515253545556575859"
      "6061626364656667686970717273747576777879"
      "8081828384858687888990919293949596979899"[value * 2];
}

// Copies two characters from src to dst.
template <typename Char>
inline constexpr void copy2(Char* dst, const char* src) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    *dst++ = static_cast<Char>(*src++);
    *dst = static_cast<Char>(*src);
  } else {
    memcpy(dst, src, 2);
  }
}

template <typename T>
  requires(std::is_unsigned_v<T>)
constexpr char* write_decimal(T abs_number, char* next) noexcept {
  // Write number from right to left.
  while (abs_number >= 100) {
    next -= 2;
    copy2(next, digits2(static_cast<size_t>(abs_number % 100)));
    abs_number /= 100;
  }
  if (abs_number < 10) {
    *--next = '0' + static_cast<char>(abs_number);
    return next;
  }
  next -= 2;
  copy2(next, digits2(static_cast<size_t>(abs_number)));
  return next;
}

template <size_t BaseBits, typename T>
  requires(std::is_unsigned_v<T>)
constexpr char* write_uint(T abs_number, const bool upper, char* next) noexcept {
  const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
  do {
    T digit = static_cast<T>(abs_number & ((1 << BaseBits) - 1));
    if constexpr (BaseBits < 4) {
      EMIO_Z_DEV_ASSERT(digit < 8);
      *--next = static_cast<char>('0' + digit);
    } else {
      EMIO_Z_DEV_ASSERT(digit < 16);
      *--next = digits[digit];
    }
  } while ((abs_number >>= BaseBits) != 0);
  return next;
}

template <typename T>
  requires(std::is_unsigned_v<T>)
constexpr char* write_number(T abs_number, const int base, const bool upper, char* next) noexcept {
  if (base == 10) {
    return write_decimal(abs_number, next);
  }
  if (base == 16) {
    return write_uint<4>(abs_number, upper, next);
  }
  if (base == 2) {
    return write_uint<1>(abs_number, false, next);
  }
  if (base == 8) {
    return write_uint<3>(abs_number, false, next);
  }
  if (abs_number == 0) {
    *(--next) = '0';
    return next;
  }
  // Write number from right to left.
  for (; abs_number; abs_number /= static_cast<T>(base)) {
    const char c = digit_to_char(static_cast<int>(abs_number % static_cast<T>(base)), upper);
    *(--next) = c;
  }
  return next;
}

inline constexpr size_t npos = std::string_view::npos;

constexpr std::string_view unchecked_substr(const std::string_view& str, size_t pos, size_t n = npos) noexcept {
  const size_t rlen = std::min(n, str.length() - pos);
  return {str.data() + pos, rlen};
}

template <typename Size>
constexpr char* fill_n(char* out, Size count, char value) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    for (Size i = 0; i < count; i++) {
      *out++ = value;
    }
    return out;
  } else {
    std::memset(out, value, to_unsigned(count));
    return out + count;
  }
}

template <typename Size>
constexpr char* copy_n(const char* in, Size count, char* out) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    for (Size i = 0; i < count; i++) {
      *out++ = *in++;
    }
    return out;
  } else {
    std::memcpy(out, in, to_unsigned(count));
    return out + count;
  }
}

[[nodiscard]] inline constexpr bool equal_n(const char* a, const char* b, const size_t n) {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    for (size_t i = 0; i < n; i++) {
      if (a[i] != b[i]) {
        return false;
      }
    }
    return true;
  } else {
    return std::memcmp(a, b, n) == 0;
  }
}

using namespace std::string_view_literals;

// Helper function to construct string literals directly as string_view during compilation if string_view_literal
// operator "" sv is not available.
inline consteval std::string_view sv(std::string_view sv) noexcept {
  return sv;
}

}  // namespace emio::detail
